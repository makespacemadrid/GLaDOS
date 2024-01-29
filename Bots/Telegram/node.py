# -*- coding: utf-8 -*-
import os
import json
import gladosMQTT
import platform
from telethon import TelegramClient, events

# Configuración de Telegram
api_id = os.environ.get("TELEGRAM_API_ID")  # Reemplaza con tu propio api_id
api_hash = os.environ.get("TELEGRAM_API_HASH")  # Reemplaza con tu propio api_hash
telegram_token = os.environ.get("TELEGRAM_BOT_TOKEN")  # Reemplaza con tu token de bot

# Configuración MQTT
mqHost	 = os.environ.get("MQTT_HOST")
mqPort 	 = os.environ.get("MQTT_PORT")
nodeName = platform.node()

# Iniciar cliente de Telegram
telegram_client = TelegramClient('telegram_bot_session', api_id, api_hash)

# Conectar con MQTT
gladosMQTT.initMQTT(mqHost, mqPort, nodeName, lambda *args: None, lambda *args: None, lambda *args: None)

# Temas MQTT para comunicarse con otros componentes
topic_telegram = "comms/telegram"
topic_glados_send_msg = topic_telegram + "/send"


def subscribeTopics() :
	gladosMQTT.subscribe(topic_glados_send_msg)


def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

def on_message(client, userdata, msg):
	if (msg.topic == topic_glados_send_msg) :
		try:
			payload = msg.payload.decode('utf-8')
			data = json.loads(payload)
			dest    = data['dest']
			content = data['msg']
			sendTelegramMsg(dest,content)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")
      
# Enviar mensajes a Telegram
def sendTelegramMsg(id, msg):
    response = json.dumps({"dest": id, "msg": msg})
    gladosMQTT.debug(f"--->Respuesta a Telegram: {response}")
    gladosMQTT.publish(topic_glados_send_msg, response)

# Evento para manejar nuevos mensajes en Telegram
@telegram_client.on(events.NewMessage)
async def handle_new_message(event):
    if event.is_private:
        msg = event.message.message
        chat_id = event.message.chat_id
        # Aquí puedes añadir lógica adicional para procesar el mensaje
        # Por ejemplo, enviarlo a otro servicio o responder directamente
        await event.respond('Recibí tu mensaje: ' + msg)  # Respuesta de ejemplo

# Iniciar el bot de Telegram
def start_telegram_bot():
    with telegram_client:
        telegram_client.run_until_disconnected()

if __name__ == "__main__":
    start_telegram_bot()
