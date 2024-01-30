# -*- coding: utf-8 -*-
import os
import platform
import json
from gladosMQTT import GladosMQTT
from flask import Flask, request, jsonify
from slack_sdk import WebClient
from slack_sdk.errors import SlackApiError

# Configuración
mqHost = os.environ.get("MQTT_HOST")
mqPort = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()
slack_token = os.environ.get("SLACK_API_TOKEN")
slack_port = os.environ.get("SLACK_PORT")

# Variables globales para mantener el estado
last_open_status = False
report_open = True

# Verificación de variables críticas
if not slack_token:
    print("Falta el token de Slack")
    exit(1)

# Temas MQTT
topic_spaceapi = "space/status" 
topic_report_space_open = "space/report_open"
topic_last_open_status = "space/last_open_status"
topic_slack = "comms/slack"
topic_slack_event = topic_slack+"/event"
topic_slack_send_msg_id = topic_slack+"/send_id"
topic_slack_send_msg_name = topic_slack+"/send_name"

# Instancia de GladosMQTT
glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName)
glados_mqtt.set_topics([topic_spaceapi, topic_last_open_status, topic_slack_send_msg_id, topic_slack_send_msg_name])

# Cliente de Slack
slack_client = WebClient(token=slack_token)
slack_client.users_setPresence(presence="auto")

# Flask app
app = Flask(__name__)

# Función para manejar mensajes MQTT
def on_mqtt_message(client, userdata, msg):
    global last_open_status
    global report_open

    # Manejar mensaje de SpaceAPI
    if msg.topic == topic_spaceapi:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            open_status = data['state']['open']
            openSpace(open_status)
        except json.JSONDecodeError as e:
            glados_mqtt.debug("Error al parsear JSON: " + str(e))

    # Manejar último estado de apertura
    elif msg.topic == topic_last_open_status:
        last_open_status = msg.payload.decode('utf-8') == 'true'

    # Manejar reporte de estado de apertura
    elif msg.topic == topic_report_space_open:
        report_open = msg.payload.decode('utf-8') == 'true'

    # Enviar mensaje a Slack por ID
    elif msg.topic == topic_slack_send_msg_id:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            sendSlackMsgbyID(data['dest'], data['msg'])
        except json.JSONDecodeError as e:
            glados_mqtt.debug("Error al procesar mensaje para Slack (ID): " + str(e))

    # Enviar mensaje a Slack por nombre
    elif msg.topic == topic_slack_send_msg_name:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            sendSlackMsgbyName(data['dest'], data['msg'])
        except json.JSONDecodeError as e:
            glados_mqtt.debug("Error al procesar mensaje para Slack (nombre): " + str(e))

# Función adicional para manejar el estado de apertura
def openSpace(status):
    global last_open_status
    global report_open

    if not report_open:
        return

    if status and not last_open_status:
        glados_mqtt.debug("Espacio abierto")
        glados_mqtt.publish(topic_last_open_status, 'true', True)
        last_open_status = True
        sendSlackMsgbyName("abierto-cerrado", "¡Espacio Abierto! Let's Make!")
    elif not status and last_open_status:
        glados_mqtt.debug("Espacio cerrado")
        glados_mqtt.publish(topic_last_open_status, 'false', True)
        last_open_status = False
        sendSlackMsgbyName("abierto-cerrado", "¡Espacio Cerrado! ZZzzZZ")

# Configuración de callbacks MQTT
glados_mqtt.mqttClient.on_message = on_mqtt_message

# Iniciar cliente de Slack
slack_client = WebClient(token=slack_token)

# Función para enviar un mensaje a Slack por ID de usuario o canal
def sendSlackMsgbyID(channel_id, msg):
    try:
        slack_client.chat_postMessage(channel=channel_id, text=msg)
    except SlackApiError as e:
        glados_mqtt.debug(f"Error al enviar mensaje a Slack por ID: {e}")

# Función para enviar un mensaje a Slack por nombre de usuario o canal
def sendSlackMsgbyName(name, msg):
    # Determinar si es un usuario o un canal y obtener su ID
    is_user = getSlackUserId(name)
    is_channel = getSlackChannelId(name) if not is_user else None
    target_id = is_user or is_channel

    if target_id:
        try:
            slack_client.chat_postMessage(channel=target_id, text=msg)
        except SlackApiError as e:
            glados_mqtt.debug(f"Error al enviar mensaje a Slack por nombre: {e}")
    else:
        glados_mqtt.debug("Usuario o canal no encontrado en Slack")

# Función para obtener el ID de un canal de Slack por su nombre
def getSlackChannelId(channel_name):
    try:
        response = slack_client.conversations_list()
        for channel in response['channels']:
            if channel['name'] == channel_name:
                return channel['id']
        return None
    except SlackApiError as e:
        glados_mqtt.debug(f"Error al obtener ID del canal de Slack: {e}")
        return None

# Función para obtener el ID de un usuario de Slack por su nombre
def getSlackUserId(user_name):
    try:
        response = slack_client.users_list()
        for user in response['members']:
            if 'name' in user and user['name'] == user_name:
                return user['id']
        return None
    except SlackApiError as e:
        glados_mqtt.debug(f"Error al obtener ID de usuario de Slack: {e}")
        return None

# Función para publicar una vista en el "home" de un usuario en Slack
def publishHomeView(user_id, view):
    try:
        slack_client.views_publish(user_id=user_id, view=view)
    except SlackApiError as e:
        glados_mqtt.debug(f"Error al publicar vista en Slack: {e}")

# Rutas Flask
@app.route('/slack/events', methods=['POST'])
def slack_events():
	data = request.json
	glados_mqtt.debug(f"Evento de Slack recibido: {data}")

	# Desafío de URL para la verificación con Slack
	if data.get('type') == 'url_verification':
		return jsonify({'challenge': data.get('challenge')})

	# Manejo de eventos de callback
	if data.get('type') == 'event_callback':
		event = data.get('event', {})
		# Ejemplo: Manejo de mensajes nuevos
		glados_mqtt.publish(topic_slack_event, json.dumps(event)) # Enviamos el evento a la cola mqtt
		if event.get('type') == 'message' and not event.get('subtype'):
			user_id = event.get('user')
			text = event.get('text')
			channel = event.get('channel')
			# Aquí puedes implementar la lógica para responder al mensaje
			# Por ejemplo, podrías enviar un mensaje de respuesta
			# sendSlackMsgbyID(channel, f"Recibido tu mensaje: {text}")
		if event.get('type') == 'app_home_opened':
			user_id = event.get('user')
			if user_id:
				publishHomeView(user_id)

	return jsonify({'status': 'ok'}), 200

if __name__ == "__main__":
    glados_mqtt.init_mqtt()
    app.run(host='0.0.0.0', port=slack_port)
