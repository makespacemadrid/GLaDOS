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


#Variables
mqHost	 = os.environ.get("MQTT_HOST", "mqtt.makespacemadrid.org")
mqPort 	 = os.environ.get("MQTT_PORT", 1883)
nodeName = platform.node()

slack_token = os.environ.get("SLACK_API_TOKEN")
slack_port  = os.environ.get("SLACK_PORT")

topic_spaceapi = "space/status" 
topic_last_open_status = "space/last_open_status"
topic_slack = "comms/slack"
topic_slack_event = topic_slack+"/event"
last_open_status = False


if not slack_token:
	print(f"Falta el token de slack {slack_token}")
	exit(1)

def subscribeTopics() :
	gladosMQTT.subscribe(topic_last_open_status)
	gladosMQTT.subscribe(topic_spaceapi)

def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

def on_message(client, userdata, msg):
	if (msg.topic == topic_spaceapi) :
		try:
			# Extraer la carga útil y decodificarla a una cadena de texto
			payload = msg.payload.decode('utf-8')
			data = json.loads(payload)
			open_status = data['state']['open']
			openSpace(open_status)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")

	elif(msg.topic == topic_last_open_status):
		if msg.payload.decode('utf-8') == 'true':
			last_open_status = True
		else:
			last_open_status = False

def openSpace(status):
	global last_open_status

	if status and not last_open_status:
		gladosMQTT.debug("open!")
		gladosMQTT.publish(topic_last_open_status,'true',True)
		last_open_status = True
		sendSlackMsg(getSlackChannelId("abierto-cerrado"),"¡Espacio Abierto! Let's Make!")
	elif not status and last_open_status :
		gladosMQTT.debug("closed!")
		gladosMQTT.publish(topic_last_open_status,'false',True)
		last_open_status = False
		sendSlackMsg(getSlackChannelId("abierto-cerrado"),"¡Espacio Cerrado! ZZzzZZ")



gladosMQTT.initMQTT(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)

slack_client = WebClient(token=slack_token)

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


def sendSlackMsg(id, msg):
	try:
		slack_client.chat_postMessage(channel=id, text=msg)
		gladosMQTT.debug("Mensaje enviado a" + id)
	except SlackApiError as e:
		gladosMQTT.debug("Error al enviar mensaje a usuario:")



#sendSlackMsg(getSlackChannelId("abierto-cerrado"),"Hola Mundo! Pronto tendre mucho que decir")



app = Flask(__name__)
@app.route('/slack/events', methods=['POST'])
def slack_events():
	data = request.json
	# Desafío de verificación de Slack
	if data.get('type') == 'url_verification':
		return jsonify({'challenge': data.get('challenge')})

	if data['type'] == 'event_callback':
		event = data['event']
		gladosMQTT.publish(topic_slack_event, json.dumps(event))
#		if 'bot_id' not in event and (event['type'] == 'message'):
#			try:
#				response = slack_client.chat_postMessage(
#					channel=event['channel'],
#					text=askGLaDOS(event['text'])
#				)
#			except SlackApiError as e:
#				print(f"Error sending message: {e.response['error']}")
	return jsonify({'status': 'ok'}), 200


app.run(host='0.0.0.0', port=slack_port)