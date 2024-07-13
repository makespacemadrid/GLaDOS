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
last_open_status = None
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
topic_slack_incoming_msg = topic_slack+"/incoming_msg"
topic_slack_send_msg_id = topic_slack+"/send_id"
topic_slack_send_msg_name = topic_slack+"/send_name"
topic_slack_edit_msg = topic_slack+"/edit_msg"

# Instancia de GladosMQTT
glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName)
glados_mqtt.set_topics([topic_last_open_status,topic_spaceapi, topic_slack_send_msg_id, topic_slack_send_msg_name,topic_slack_edit_msg])

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

    elif msg.topic == topic_slack_edit_msg:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            editSlackMsg(data['channel_id'], data['reply_msg_id'], data['message'])
        except json.JSONDecodeError as e:
            glados_mqtt.debug(f"Error processing message edit request: {e}")

# Función adicional para manejar el estado de apertura
def openSpace(status):
    global last_open_status
    global report_open
    if last_open_status is None :
        last_open_status = status == 'false' #Si por alguna razon last_status no esta inicializado cargamos el valor contratio para que si mande el mensaje.

    if status and not last_open_status:
        glados_mqtt.debug("Espacio abierto")
        glados_mqtt.publish(topic_last_open_status, 'true', True)
        last_open_status = True
        if report_open :
            sendSlackMsgbyName("abierto-cerrado", "¡Espacio Abierto! Let's Make!")
    elif not status and last_open_status:
        glados_mqtt.debug("Espacio cerrado")
        glados_mqtt.publish(topic_last_open_status, 'false', True)
        last_open_status = False
        if report_open :
            sendSlackMsgbyName("abierto-cerrado", "¡Espacio Cerrado! ZZzzZZ")


# Configuración de callbacks MQTT
glados_mqtt.mqttClient.on_message = on_mqtt_message

# Iniciar cliente de Slack
slack_client = WebClient(token=slack_token)

# Variables para almacenar la lista de canales y usuarios en caché
cached_slack_channels = {}
cached_slack_users = {}

# Función para cargar y almacenar en caché los canales de Slack
def cache_slack_channels():
    global cached_slack_channels
    try:
        response = slack_client.conversations_list()
        cached_slack_channels = {channel['id']: channel['name'] for channel in response['channels']}
    except SlackApiError as e:
        glados_mqtt.debug(f"Error getting Slack channel list: {e}")

# Función para cargar y almacenar en caché los usuarios de Slack
def cache_slack_users():
    global cached_slack_users
    try:
        response = slack_client.users_list()
        cached_slack_users = {user['id']: user['name'] for user in response['members'] if 'id' in user}
    except SlackApiError as e:
        glados_mqtt.debug(f"Error obtaining Slack user list: {e}")

# Función para obtener los mensajes de un hilo en Slack utilizando slack_sdk
def getThreadMessages(channel_id, thread_id):
    try:
        # Llamar al método conversations.replies para obtener los mensajes del hilo
        response = slack_client.conversations_replies(channel=channel_id, ts=thread_id)
        
        # Extraer los mensajes del hilo
        messages = response['messages']
        return messages
    except SlackApiError as e:
        print(f"Error al obtener los mensajes del hilo de Slack: {e.response['error']}")
    return []


def getEventInfo(event):
    try:
        if not isinstance(event, str):
            event = json.dumps(event)
        data = json.loads(event)

        sender_id = data.get('user')
        channel_id = data.get('channel')
        message = data.get('text')
        message_id = data.get('ts')
        thread_id = data.get('thread_ts')
        username = getSlackUserName(sender_id)
        channel_name = getSlackChannelName(channel_id)

        payload = {
            'sender_id': sender_id,
            'channel_id': channel_id,
            'username': username,
            'channel_name': channel_name,
            'message': message,
            'message_id': message_id,
            'thread_id': thread_id
        }
        return payload
    except json.JSONDecodeError as e:
        glados_mqtt.debug("Error al procesar mensaje para Slack (nombre): " + str(e))
        return False


def editSlackMsg(channel_id, message_timestamp, new_msg):
    try:
        slack_client.chat_update(channel=channel_id, ts=message_timestamp, text=new_msg)
    except SlackApiError as e:
        glados_mqtt.debug(f"Error editing message in Slack: {e}")

def processThreadMessages(channel_id, thread_id):
    # Directly get the thread messages without json.loads since getThreadMessages already returns a list
    thread_messages = getThreadMessages(channel_id, thread_id)
    processed_messages = []
    for message in thread_messages:
        payload = getEventInfo(message)
        processed_messages.append(payload)
    
    return processed_messages

# Función para procesar los eventos entrantes de Slack
def processSlackEvents(event):
    try:
        if not isinstance(event, str):
            event = json.dumps(event)
        data = json.loads(event)
        subtype = ""
        if 'subtype' in data :
            subtype = data['subtype']
        if (
            data['type'] != "message" or 
            'bot_id' in data or 
            'app_id' in data or
            subtype in ("thread_broadcast", "message_changed", "message_deleted")
        ):
            return False
        if data['channel_type'] == "channel":
            respondTo = data['channel']
            msg = data['text']
            # Si es un mensaje en un canal pero no se nos menciona no respondemos.
            if not ('<@U05LXTJ7Q66>' in msg or 'glados' in msg.lower()):
                return

        payload = getEventInfo(event)
        # Send a reply that we are 'Working' on it
        response = slack_client.chat_postMessage(channel=payload['channel_id'], text="Recibido, consultando LLM...",
                                                 thread_ts=payload['message_id'] if payload.get('thread_id') else None)
        # Extract and publish the message id of the reply
        payload['reply_msg_id']= response['message']['ts']

        # Si el mensaje está en un hilo, obtener el hilo completo
        if payload['thread_id'] :
            thread_messages = processThreadMessages(payload['channel_id'], payload['thread_id'])
            payload['thread_messages'] = thread_messages

        glados_mqtt.publish(topic_slack_incoming_msg, json.dumps(payload))
    except json.JSONDecodeError as e:
        glados_mqtt.debug("Error al procesar mensaje para Slack (nombre): " + str(e))
        return False

# Función para enviar un mensaje a Slack por ID de usuario o canal
def sendSlackMsgbyID(channel_id, msg):
    try:
        slack_client.chat_postMessage(channel=channel_id, text=msg)
    except SlackApiError as e:
        glados_mqtt.debug(f"Error al enviar mensaje a Slack por ID: {e}")

# Función para enviar un mensaje a Slack por nombre de usuario o canal
def sendSlackMsgbyName(name, msg):
    # Determinar si es un usuario o un canal y obtener su ID
    is_user = getSlackUserName(name)
    is_channel = getSlackChannelName(name) if not is_user else None
    target_id = is_user or is_channel

    if target_id:
        try:
            slack_client.chat_postMessage(channel=target_id, text=msg)
        except SlackApiError as e:
            glados_mqtt.debug(f"Error al enviar mensaje a Slack por nombre: {e}")
    else:
        glados_mqtt.debug("Usuario o canal no encontrado en Slack")

# Actualizado para usar caché
def getSlackChannelName(channel_id):
    if channel_id in cached_slack_channels:
        return cached_slack_channels[channel_id]
    else:
        # Recargar caché si el canal no se encuentra
        cache_slack_channels()
        return cached_slack_channels.get(channel_id, None)

# Actualizado para usar caché
def getSlackUserName(user_id):
    if user_id in cached_slack_users:
        return cached_slack_users[user_id]
    else:
        # Recargar caché si el usuario no se encuentra
        cache_slack_users()
        return cached_slack_users.get(user_id, None)

# Función para publicar una vista en el "home" de un usuario en Slack
def publishHomeView(user_id, view):
    try:
        slack_client.views_publish(user_id=user_id, view=view)
    except SlackApiError as e:
        glados_mqtt.debug(f"Error al publicar vista en Slack: {e}")


# Rutas Flask
@app.route('/slack/events', methods=['POST'])
def slack_events():
    try:
        data = request.json
        # Desafío de URL para la verificación con Slack
        if data.get('type') == 'url_verification':
            return jsonify({'challenge': data.get('challenge')}), 200

    # Manejo de eventos de callback
        if data.get('type') == 'event_callback':
            event = data.get('event', {})
            glados_mqtt.publish(topic_slack_event, json.dumps(event))  # Enviamos el evento a la cola mqtt
            processSlackEvents(event)

            if event.get('type') == 'app_home_opened':
                user_id = event.get('user')
                if user_id:
                    publishHomeView(user_id)

    except Exception as e:
        glados_mqtt.debug(f'Error procesando el evento de slack: {e}')
    finally:
        # Usamos el bloque finally para asegurarnos de que siempre ejecutamos el retorno.
        return jsonify({'status': 'ok'}), 200

if __name__ == "__main__":
    # Cargar listas en caché antes de iniciar el MQTT y Flask
    cache_slack_channels()
    cache_slack_users()
    glados_mqtt.init_mqtt()
    app.run(host='0.0.0.0', port=slack_port)