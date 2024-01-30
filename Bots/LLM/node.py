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
mqHost	 = str(os.environ.get("MQTT_HOST"))
mqPort 	 = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()


gladosBot = GladosIA.GladosBot()



if not mqHost or not mqPort:
	print("No mqtt config!")
	exit(1)

topic_spaceapi = "space/status" 

topic_slack = "comms/slack"
topic_slack_event = topic_slack+"/event"
topic_slack_send_msg_id = topic_slack+"/send_id"
topic_slack_send_msg_name = topic_slack+"/send_name"

topic_telegram = "comms/telegram"
topic_telegram_event = topic_telegram+"/event"
topic_telegram_send_msg_id = topic_telegram+"/send_id"

topic_discord = "comms/discord"
topic_discord_event = topic_discord+"/event"
topic_discord_send_msg_id = topic_discord+"/send_id"

def subscribeTopics() :
	gladosMQTT.subscribe(topic_spaceapi)
	gladosMQTT.subscribe(topic_slack_event)
	gladosMQTT.subscribe(topic_telegram_event)
	gladosMQTT.subscribe(topic_discord_event)


def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

def on_message(client, userdata, msg):
	payload = msg.payload.decode('utf-8')
	gladosMQTT.debug(f'TOPIC> {msg.topic} payload: {payload}')
	if (msg.topic == topic_spaceapi) :
		try:
			# Extraer la carga útil y decodificarla a una cadena de texto
			payload = msg.payload.decode('utf-8')
			data = json.loads(payload)
			open_status = data['state']['open']
			#mandar mensaje de apertura con el llm?
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")
	elif(msg.topic == topic_slack_event):
		try:
			# Extraer la carga útil y decodificarla a una cadena de texto
			payload = msg.payload.decode('utf-8')
			processSlackEvent(payload)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")
	elif(msg.topic == topic_telegram_event):
		try:
			# Extraer la carga útil y decodificarla a una cadena de texto
			payload = msg.payload.decode('utf-8')
			processTelegramEvent(payload)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")
	elif(msg.topic == topic_discord_event):
		try:
			# Extraer la carga útil y decodificarla a una cadena de texto
			payload = msg.payload.decode('utf-8')
			processDiscordEvent(payload)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")

#SLACK EVENT
def processSlackEvent(event):
	gladosMQTT.debug("--->SLACK event ------------------")
	gladosMQTT.debug(event)
	gladosMQTT.debug("/SLACK event ------------------")
	try:
		data = json.loads(event)
		if data['type'] != "message" or 'bot_id' in data:
			return False
	except:
		gladosMQTT.debug("processSlackEvent:Error procesado json")
		return False

#	try:
			# Mensajes de union a canal
	if 'subtype' in data and data['subtype']=="channel_join":
		respondTo = data['channel']
		msg = data['text']
		response = gladosBot.ask(msg,respondTo)
		sendToSlack(respondTo,response)					
	#Mensaje  a canal 
	elif data['channel_type'] == "channel":
		respondTo = data['channel']
		msg = data['text']
		if '<@U05LXTJ7Q66>' in msg or 'glados' in msg.lower():
			#sendToSlack(respondTo,"Pensando... dame unos segundos")	
			response = gladosBot.ask(msg,respondTo)
			sendToSlack(respondTo,response)	
	#Mensaje privado
	elif data['channel_type'] == "im":
		respondTo = data['user']
		msg = data['text']
		response = gladosBot.ask(msg,respondTo)
		#sendToSlack(respondTo,"Pensando... dame unos segundos")
		sendToSlack(respondTo,response)
#	except Exception as e:
#		error_message = f"processSlackEvent: Error gestionando evento - {str(e)}"
#		gladosMQTT.debug(error_message)
#		sendToSlack(respondTo, f"ERROR: algo no ha funcionado :S - {str(e)}")


def processTelegramEvent(event):
	gladosMQTT.debug("--->TELEGRAM event ------------------")
	gladosMQTT.debug(event)
	gladosMQTT.debug("/TELEGRAM event ------------------")
	try:
		data = json.loads(event)
		respondTo=data['sender_id']
		msg=data['message_text']
		response = gladosBot.ask(msg,respondTo)
		sendToTelegram(respondTo,response)	
	except:
		gladosMQTT.debug("processTelegramEvent:Error procesado json")
		return False


def processDiscordEvent(event):
	gladosMQTT.debug("--->DISCORD event ------------------")
	gladosMQTT.debug(event)
	gladosMQTT.debug("/Discord event ------------------")
	#try:
	data = json.loads(event)
	respondTo=data['sender_id']
	msg=data['message_text']
	response = gladosBot.ask(msg,respondTo)
	sendToDiscord(respondTo,response)	
	#except:
	#	gladosMQTT.debug("processDiscordEvent:Error procesado json")
	#	return False

#SEND TO SLACK
def sendToSlack(id,msg):
	response = json.dumps({"dest": id, "msg": msg})
	gladosMQTT.debug(f"--->Respuesta a slack: {response}")
	gladosMQTT.publish(topic_slack_send_msg_id,response)

def sendToTelegram(id,msg):
	response = json.dumps({"dest": id, "msg": msg})
	gladosMQTT.debug(f"--->Respuesta a slack: {response}")
	gladosMQTT.publish(topic_telegram_send_msg_id,response)

def sendToDiscord(id,msg):
	response = json.dumps({"dest": id, "msg": msg})
	gladosMQTT.debug(f"--->Respuesta a slack: {response}")
	gladosMQTT.publish(topic_discord_send_msg_id,response)


gladosMQTT.initMQTTandLoopForever(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)