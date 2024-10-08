# -*- coding: utf-8 -*-

#Nodo Mqtt de ejemplo, 
#gladosMQTT.py contiene la funcionalidad para manejar el mqtt.
# aui nos conectamos y reaccionamos a los mensajes que llegan.
# TODO Documentar mejor...  gpt? xD

import os

import gladosMQTT
#import Bots.LLM.llm as llm
import GladosIA
import platform
import time
import json



#Variables
#mqHost	 = os.environ.get("MQTT_HOST", "mqtt.makespacemadrid.org")
#mqPort 	 = os.environ.get("MQTT_PORT", 1883)
mqHost	 = "mqtt.makespacemadrid.org" #WTF! No entiendo que pasa con las variables de entorno que vienen del compose :S
mqPort 	 = 1883
nodeName = platform.node()


gladosBot = GladosIA.GladosBot()



if not mqHost or not mqPort:
	print("No mqtt config!")
	exit(1)

topic_spaceapi = "space/status" 
topic_slack = "comms/slack"
topic_slack_event = topic_slack+"/event"
topic_glados_send_msg_id = topic_slack+"/send_id"
topic_glados_send_msg_name = topic_slack+"/send_name"


def subscribeTopics() :
	gladosMQTT.subscribe(topic_spaceapi)
	gladosMQTT.subscribe(topic_slack_event)

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
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")

	elif(msg.topic == topic_slack_event):
		try:
			# Extraer la carga útil y decodificarla a una cadena de texto
			payload = msg.payload.decode('utf-8')
			processSlackEvent(payload)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")


def processSlackEvent(event):
	gladosMQTT.debug(event)
	try:
		data = json.loads(event)
		if data['type'] != "message" or 'bot_id' in data:
			return False
	except:
		gladosMQTT.debug("processSlackEvent:Error procesado json")

	try:
			# Mensajes de union a canal
			if 'subtype' in data and data['subtype']=="channel_join":
				respondTo = data['channel']
				msg = data['text']
				response = gladosBot.ask(msg)
				gladosMQTT.debug(response)
				sendToSlack(respondTo,response)					
			#Mensaje  a canal 
			elif data['channel_type'] == "channel":
				respondTo = data['channel']
				msg = data['text']
				if '<@U05LXTJ7Q66>' in msg or 'glados' in msg.lower():
					response = gladosBot.ask(msg)
					gladosMQTT.debug(response)
					sendToSlack(respondTo,response)	
			#Mensaje privado
			elif data['channel_type'] == "im":
				respondTo = data['user']
				msg = data['text']
				response = gladosBot.ask(msg)
				gladosMQTT.debug(response)
				sendToSlack(respondTo,response)
	except:
		gladosMQTT.debug("processSlackEvent:Error gestionando evento")
		sendToSlack(respondTo,"ERROR: Computer says nooooooo")

def sendToSlack(id,msg):
	response = json.dumps({"dest": id, "msg": msg})
	gladosMQTT.debug(f"Respuesta a slack: {response}")
	gladosMQTT.publish(topic_glados_send_msg_id,response)

gladosMQTT.initMQTTandLoopForever(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)