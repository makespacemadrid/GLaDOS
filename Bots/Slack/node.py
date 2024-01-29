# -*- coding: utf-8 -*-

#Nodo Mqtt de ejemplo, 
#gladosMQTT.py contiene la funcionalidad para manejar el mqtt.
# aui nos conectamos y reaccionamos a los mensajes que llegan.
# TODO Documentar mejor...  gpt? xD

import os

import gladosMQTT
import platform
import time
import json


from flask import Flask, request, jsonify
from slack_sdk import WebClient
from slack_sdk.errors import SlackApiError

InitialHome=False
#Variables
mqHost	 = os.environ.get("MQTT_HOST", "mqtt.makespacemadrid.org")
mqPort 	 = os.environ.get("MQTT_PORT", 1883)
nodeName = platform.node()

slack_token = os.environ.get("SLACK_API_TOKEN")
slack_port  = os.environ.get("SLACK_PORT")

last_open_status = False
report_open = True


topic_spaceapi = "space/status" 
topic_report_space_open = "space/report_open"
topic_last_open_status = "space/last_open_status"
topic_slack = "comms/slack"
topic_slack_event = topic_slack+"/event"
topic_glados_send_msg_id = topic_slack+"/send_id"
topic_glados_send_msg_name = topic_slack+"/send_name"


if not slack_token:
	print(f"Falta el token de slack {slack_token}")
	exit(1)

def subscribeTopics() :
	gladosMQTT.subscribe(topic_last_open_status)
	gladosMQTT.subscribe(topic_spaceapi)
	gladosMQTT.subscribe(topic_glados_send_msg_id)
	gladosMQTT.subscribe(topic_glados_send_msg_name)

def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

def on_message(client, userdata, msg):
	global last_open_status
	global report_open

#SpaceAPI
	if (msg.topic == topic_spaceapi) :
		try:
			# Extraer la carga útil y decodificarla a una cadena de texto
			payload = msg.payload.decode('utf-8')
			data = json.loads(payload)
			open_status = data['state']['open']
			openSpace(open_status)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")
#LastOpenStatus
	elif(msg.topic == topic_last_open_status):
		if msg.payload.decode('utf-8') == 'true':
			last_open_status = True
		else:
			last_open_status = False
#ReportSpaceOpen
	elif(msg.topic == topic_report_space_open):
		if msg.payload.decode('utf-8') == 'true':
			report_open = True
		else:
			report_open = False
#SendMsgID
	elif(msg.topic == topic_glados_send_msg_id):
		payload = msg.payload.decode('utf-8')
		data = json.loads(payload)
		dest    = data['dest']
		content = data['msg']
		sendSlackMsgbyID(dest,content)
#SendMsgName
	elif(msg.topic == topic_glados_send_msg_id):
		payload = msg.payload.decode('utf-8')
		data = json.loads(payload)
		dest    = data['dest']
		content = data['msg']
		sendSlackMsgbyName(dest,content)

def openSpace(status):
	global last_open_status
	global report_open

	if not report_open:
		return

	if status and not last_open_status:
		gladosMQTT.debug("open!")
		gladosMQTT.publish(topic_last_open_status,'true',True)
		last_open_status = True
		sendSlackMsgbyName("abierto-cerrado","¡Espacio Abierto! Let's Make!")
	elif not status and last_open_status :
		gladosMQTT.debug("closed!")
		gladosMQTT.publish(topic_last_open_status,'false',True)
		last_open_status = False
		sendSlackMsgbyName("abierto-cerrado","¡Espacio Cerrado! ZZzzZZ")



#Funciones SLACK

def set_bot_status(status_text, status_emoji=":robot_face:"):
    try:
        slack_client.users_profile_set(
            profile={
                "status_text": status_text,
                "status_emoji": status_emoji,
                "status_expiration": 0
            }
        )
    except SlackApiError as e:
        print(f"Error al establecer el estado del bot: {e}")



def publishHomeView(user_id):
	home_view = {
		"type": "home",
		"blocks": [
			{
				"type": "section",
				"text": {
					"type": "mrkdwn",
					"text": "*Hola, me llamo GLaDOS y soy la ia generativa de makespace!*\nPreguntame por privado en la pestana mensajes o mencioname en cualquier canal de los que participo"
				}
			}
		]
	}
	try:
		slack_client.views_publish(
			user_id=user_id,
			view=home_view
		)
		print("Pantalla de inicio publicada con éxito")
	except SlackApiError as e:
		print(f"Error al publicar la pantalla de inicio: {e}")


def getSlackChannelId(name):
	try:
		channels_response = slack_client.conversations_list()
		for channel in channels_response['channels']:
			gladosMQTT.debug(channel['name'])
			if channel['name'] == name:
				return channel['id']
			print("Canal no encontrado")
		return None
	except SlackApiError as e:
		print(f"Error al obtener el ID del canal: {e}")
		return None

def getSlackUserId(username):
    try:
        response = slack_client.users_list()
        users = response['members']
        for user in users:
            if 'name' in user and user['name'] == username:
                return user['id']
        return None
    except SlackApiError as e:
        gladosMQTT.debug("Error al obtener la lista de usuarios")
        return None


def sendSlackMsgbyID(id, msg):
	try:
		slack_client.chat_postMessage(channel=id, text=msg)
		gladosMQTT.debug("Mensaje enviado a" + id)
	except SlackApiError as e:
		gladosMQTT.debug("Error al enviar mensaje a usuario:")

def sendSlackMsgbyName(name, msg):
	try:
		#Es un usuario?
		isuser = getSlackUserId(name)
		if isuser:
			slack_client.chat_postMessage(channel=isuser, text=msg)
			gladosMQTT.debug(f"Mensaje enviado a {isuser}")
			return 0
		#Es un canal?
		ischannel = getSlackChannelId(name)
		if ischannel:
			slack_client.chat_postMessage(channel=ischannel, text=msg)
			gladosMQTT.debug(f"Mensaje enviado a {isuser}")
			return 0
	except SlackApiError as e:
		gladosMQTT.debug(f"Error al enviar mensaje a usuario: {e}")
	#Si llegamos aqui es que no hemos podido encontrarlo en la lista de usuarios ni en la de canales
	gladosMQTT.debug(f"Error al enviar mensaje: no se encuentra el usuario o canal de destino")




#Iniciar app
	
gladosMQTT.initMQTT(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)

slack_client = WebClient(token=slack_token)
slack_client.users_setPresence(presence="auto")
set_bot_status("Boot")

app = Flask(__name__)
publishHomeView("U0A7VU47Q")

@app.route('/slack/events', methods=['POST'])
def slack_events():
	data = request.json
	gladosMQTT.debug(f"SLACK EVENT: {json.dumps(data)}")
	# Desafío de verificación de Slack
	if data.get('type') == 'url_verification':
		return jsonify({'challenge': data.get('challenge')})

	if data['type'] == 'event_callback':
		event = data['event']
		set_bot_status('Processing')
		gladosMQTT.publish(topic_slack_event, json.dumps(event))

		if event.get('type') == 'app_home_opened':
			user_id = event.get('user')
			if user_id:
				publishHomeView(user_id)
	set_bot_status('Idle')
	return jsonify({'status': 'ok'}), 200


app.run(host='0.0.0.0', port=slack_port)