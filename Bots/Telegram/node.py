# -*- coding: utf-8 -*-
import os
import json
import gladosMQTT
import platform
from telethon import TelegramClient, events

#ENV
from dotenv import load_dotenv
load_dotenv()

# Configuración de Telegram
api_id = os.environ.get("TELEGRAM_API_ID")  # Reemplaza con tu propio api_id
api_hash = os.environ.get("TELEGRAM_API_HASH")  # Reemplaza con tu propio api_hash
telegram_token = os.environ.get("TELEGRAM_BOT_TOKEN")  # Reemplaza con tu token de bot

# Configuración MQTT
#Variables
mqHost	 = os.environ.get("MQTT_HOST", "mqtt.makespacemadrid.org") #WTF! No entiendo que pasa con las variables de entorno que vienen del compose :S
mqPort 	 = os.environ.get("MQTT_PORT", 1883)
nodeName = platform.node()


# Temas MQTT para comunicarse con otros componentes
topic_telegram = "comms/telegram"
topic_telegram_send_msg = topic_telegram + "/send"
topic_telegram_event = topic_telegram + "/event"


# Iniciar cliente de Telegram
telegram_client = TelegramClient('/tmp/telegram_bot_session', api_id, api_hash)

def subscribeTopics() :
	gladosMQTT.subscribe(topic_telegram_send_msg)


def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

def on_message(client, userdata, msg):
	if (msg.topic == topic_telegram_send_msg) :
		try:
			payload = msg.payload.decode('utf-8')
			data = json.loads(payload)
			dest    = data['dest']
			content = data['msg']
			sendTelegramMsg(dest,content)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")
      
# Enviar mensajes a Telegram
async def sendTelegramMsg(user_id, msg):
    gladosMQTT.debug(f"--->Respuesta a Telegram: {user_id} : {msg}")
    await telegram_client.send_message(user_id, msg)    

# Evento para manejar nuevos mensajes en Telegram
@telegram_client.on(events.NewMessage)
async def handle_new_message(event):
    gladosMQTT.publish(topic_telegram_event, json.dumps(event)) 
    if event.is_private:
        msg = event.message.message
        chat_id = event.message.chat_id
        await event.respond('Recibí tu mensaje: ' + msg)  # Respuesta de ejemplo

# Iniciar el bot de Telegram
def start_telegram_bot():
    with telegram_client:
        telegram_client.run_until_disconnected()



if __name__ == "__main__":
    gladosMQTT.initMQTT(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)
    start_telegram_bot()
