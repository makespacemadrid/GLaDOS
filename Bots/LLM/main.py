# -*- coding: utf-8 -*-

import os
import platform
import json
from gladosMQTT import GladosMQTT
import GladosIA

# Configuración MQTT
mqHost = os.environ.get("MQTT_HOST")
mqPort = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()

# Verificación de variables críticas
if not mqHost or not mqPort:
    print("No MQTT config!")
    exit(1)

# Instancia de GladosIA
gladosBot = GladosIA.GladosBot()

# Temas MQTT
topic_spaceapi = "space/status"
topic_slack_event = "comms/slack/event"
topic_slack_incoming_msg = "comms/slack/incoming_msg"
topic_telegram_event = "comms/telegram/event"
topic_discord_event = "comms/discord/event"
topic_slack_send_msg_id = "comms/slack/send_id"
topic_slack_edit_msg = "comms/slack/edit_msg"
topic_telegram_send_msg_id ="comms/telegram/send_id"
topic_telegram_edit_msg    ="comms/telegram/edit_msg"
topic_discord_send_msg_id ="comms/discord/send_id"

# Instancia de GladosMQTT
glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName)
glados_mqtt.set_topics([topic_spaceapi, topic_slack_event, topic_slack_incoming_msg, topic_telegram_event, topic_discord_event])

# Función para manejar mensajes MQTT
def on_mqtt_message(client, userdata, msg):
	payload = msg.payload.decode('utf-8')
	if msg.topic == topic_spaceapi:
		data = json.loads(payload)
		open_status = data['state']['open']
		#mandar mensaje de apertura con el llm?
#	elif msg.topic == topic_slack_event:
#		processSlackEvent(payload)
	elif msg.topic == topic_slack_incoming_msg:
		processSlackMSG(payload)
	elif msg.topic == topic_telegram_event:
		processTelegramEvent(payload)
	elif msg.topic == topic_discord_event:
		processDiscordEvent(payload)
	# Añade más condiciones según sea necesario

# Configurar callbacks MQTT
glados_mqtt.mqttClient.on_message = on_mqtt_message


def processSlackMSG(event) :
	if not isinstance(event, str):
		event = json.dumps(event)
	data = json.loads(event)

	sender_id    = data.get('sender_id')
	channel_id   = data.get('channel_id')
	message      = data.get('message')
	message_id   = data.get('message_id')
	thread_id    = data.get('thread_id')
	username     = data.get('username')
	channel_name = data.get('channel_name')
	reply_msg_id = data.get('reply_msg_id')

	response = gladosBot.ask(message, sender_id)

	payload = {
		'sender_id': sender_id,
		'channel_id': channel_id,
		'message': response,
		'reply_msg_id': reply_msg_id
	}
	glados_mqtt.publish(topic_slack_edit_msg, json.dumps(payload))


# Funciones para procesar eventos de Slack, Telegram y Discord

def processTelegramEvent(event):
 #   try:
	data = json.loads(event)
	respondTo = data['sender_id']
	msg = data['message_text']
	response = gladosBot.ask(msg, respondTo)
	payload = {
		'sender_id': data['sender_id'],
		'channel_id': data['channel_id'],
		'message_text': response,
		'reply_msg_id': data['reply_msg_id']
	}
	
	glados_mqtt.publish(topic_telegram_edit_msg, json.dumps(payload))

#	sendToTelegram(respondTo, response)
 #   except:
 #       glados_mqtt.debug("processTelegramEvent: Error procesando JSON")
 #       return False


def processDiscordEvent(event):
	glados_mqtt.debug("---> DISCORD event ------------------")
	glados_mqtt.debug(event)
	glados_mqtt.debug("/ DISCORD event ------------------")
	#   try:
	data = json.loads(event)
	respondTo = str(data['channel_id'])
	msg = data['content']
	response = gladosBot.ask(msg, respondTo)
	sendToDiscord(respondTo, response)
 #   except:
 #       glados_mqtt.debug("processDiscordEvent: Error procesando JSON")
 #       return False

# Funciones para enviar respuestas a Slack, Telegram y Discord
def sendToSlack(id, msg):
    response = json.dumps({"dest": id, "msg": msg})
    glados_mqtt.debug(f"---> Respuesta a Slack: {response}")
    glados_mqtt.publish(topic_slack_send_msg_id, response)

def sendToTelegram(id, msg):
    response = json.dumps({"dest": id, "msg": msg})
    glados_mqtt.debug(f"---> Respuesta a Telegram: {response}")
    glados_mqtt.publish(topic_telegram_send_msg_id, response)

# Enviar mensaje a Discord por ID de usuario o canal
def sendToDiscord(id, msg):
	response = json.dumps({"dest": id , "msg": msg})
	glados_mqtt.debug(f'WTF id: {id}  response; {response}')
	glados_mqtt.debug(f"---> Respuesta a Discord: {response}")
	glados_mqtt.publish(topic_discord_send_msg_id, response)


if __name__ == "__main__":
    glados_mqtt.init_mqtt_and_loop_forever()
