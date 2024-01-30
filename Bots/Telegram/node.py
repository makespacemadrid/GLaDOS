# -*- coding: utf-8 -*-
import os
import json
import gladosMQTT
import platform
from telethon import TelegramClient, events
import asyncio
import requests


# Configuración de Telegram
api_id = os.environ.get("TELEGRAM_API_ID")  # Reemplaza con tu propio api_id
api_hash = os.environ.get("TELEGRAM_API_HASH")  # Reemplaza con tu propio api_hash
telegram_token = os.environ.get("TELEGRAM_BOT_TOKEN") # Reemplaza con tu token de bot

# Variable global para almacenar el ID del bot
bot_id = None

# Configuración MQTT
#Variables
mqHost	 = str(os.environ.get("MQTT_HOST")) #WTF! No entiendo que pasa con las variables de entorno que vienen del compose :S
mqPort 	 = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()

if not telegram_token:
    raise ValueError("No se encontró el token del bot de Telegram.")
if not api_hash:
    raise ValueError("No se encontró el api_hash del bot de Telegram.")
if not api_id:
    raise ValueError("No se encontró el api_id del bot de Telegram.")
if not nodeName:
    raise ValueError("No se encontró el nodeName del bot de Telegram.")
if not mqHost:
    raise ValueError("No se encontró el mqHost del bot de Telegram.")
if not mqPort:
    raise ValueError("No se encontró el mqPort del bot de Telegram.")


# Temas MQTT para comunicarse con otros componentes
topic_telegram = "comms/telegram"
topic_telegram_send_msg = topic_telegram + "/send_id"
topic_telegram_event = topic_telegram + "/event"


# Iniciar cliente de Telegram
telegram_client = TelegramClient('/data/tg_sess.ignore', api_id, api_hash)


async def send_audio_to_transcription_api(audio_file_path):
    url = "https://whisper.makespacemadrid.org/asr"
    params = {
        "encode": "true",
        "task": "transcribe",
        "language": "es",
        "word_timestamps": "false",
        "output": "txt"
    }
    files = {'audio_file': (audio_file_path, open(audio_file_path, 'rb'), 'audio/wav')}

    try:
        response = requests.post(url, params=params, files=files)
        response.raise_for_status()  # Esto provocará una excepción si la respuesta no es exitosa
        return response.text  # Retorna el contenido de la transcripción
    except requests.RequestException as e:
        return(f"Error al enviar el archivo de audio a la API: {e}")



def subscribeTopics() :
	gladosMQTT.subscribe(topic_telegram_send_msg)


def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))


loop = asyncio.get_event_loop()


def send_telegram_message_sync(user_id, message):
    asyncio.run_coroutine_threadsafe(telegram_client.send_message(user_id, message), loop)




def on_message(client, userdata, msg):
    if msg.topic == topic_telegram_send_msg:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            dest = data['dest']
            content = data['msg']

            # Llama al callback síncrono
            send_telegram_message_sync(dest, content)
        except Exception as e:
            gladosMQTT.debug(f"Error al enviar mensaje: {e}")

 





# Evento para manejar nuevos mensajes en Telegram
@telegram_client.on(events.NewMessage)
async def handle_new_message(event):
    gladosMQTT.debug(event.stringify())
#    global bot_id
    # Si no se ha establecido el ID del bot, obténgalo
#    if bot_id is None:
#        me = await telegram_client.get_me()
#        bot_id = me.id
    if event.message.voice or event.message.audio:
        # Descargar el archivo de audio
        audio_file_path = await telegram_client.download_media(event.message, file='/tmp/audio/')
        # Procesar el archivo de audio
        transcription = await send_audio_to_transcription_api(audio_file_path)
        event_data = {
            'message_text': transcription,
            'sender_id': event.sender_id,
            'chat_id': event.chat_id,
            # Agrega aquí otros campos relevantes
        }
        gladosMQTT.publish(topic_telegram_event, json.dumps(event_data))
        return

    if event.is_private and event.sender_id != 771352834 :
#        await event.respond('Procesando...')
        event_data = {
            'message_text': event.message.message if event.message else None,
            'sender_id': event.sender_id,
            'chat_id': event.chat_id,
            # Agrega aquí otros campos relevantes
        }
        gladosMQTT.publish(topic_telegram_event, json.dumps(event_data))



if __name__ == "__main__":
    gladosMQTT.initMQTT(mqHost, mqPort, nodeName, on_connect, on_message, on_disconnect)
    telegram_client.start(bot_token=telegram_token)
    loop.run_until_complete(telegram_client.run_until_disconnected())

