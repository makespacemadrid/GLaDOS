# -*- coding: utf-8 -*-
import os
import json
import platform
from gladosMQTT import GladosMQTT  # Asegúrate de que el import sea correcto
from telethon import TelegramClient, events
import asyncio
import requests
from io import BytesIO


# Configuración de Telegram
api_id = os.environ.get("TELEGRAM_API_ID")
api_hash = os.environ.get("TELEGRAM_API_HASH")
telegram_token = os.environ.get("TELEGRAM_BOT_TOKEN")

# Configuración MQTT
mqHost = os.environ.get("MQTT_HOST")
mqPort = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()

tts_url = os.environ.get("TTS_URL")


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

# Temas MQTT
topic_telegram = "comms/telegram"
topic_telegram_edit_msg = topic_telegram + "/edit_msg"
topic_telegram_send_msg = topic_telegram + "/send_id"
topic_telegram_event = topic_telegram + "/event"

# Definición de los callbacks para MQTT
def on_mqtt_message(client, userdata, msg):
    if msg.topic == topic_telegram_send_msg:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            dest = data['dest']
            content = data['msg']
            send_telegram_message_sync(dest, content)
            generate_and_send_audio_response(dest,content)
        except Exception as e:
            glados_mqtt.debug(f"Error al enviar mensaje: {e}")
    elif msg.topic == topic_telegram_edit_msg:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            channel_id = data['channel_id']
            reply_id = data['reply_msg_id']
            content = data['message_text']
            edit_telegram_message_sync(channel_id,reply_id,content)
            generate_and_send_audio_response(channel_id,content)
        except Exception as e:
            glados_mqtt.debug(f"Error al editar mensaje: {e}")

def generate_and_send_audio_response(user_id, text):
    global tts_url
    headers = {'Content-Type': 'application/json'}
    data = {
        "text": text,
        "language": "es"
    }

    try:
        # Cambia a usar requests.post y envía los datos como JSON en el cuerpo de la petición
        tts_response = requests.post(tts_url, json=data, headers=headers)
        tts_response.raise_for_status()  # Esto provocará una excepción si la respuesta no es exitosa

        # Enviar el audio directamente a Telegram sin guardar en disco
        audio_bytes = BytesIO(tts_response.content)
        audio_bytes.name = "response.wav"
        send_telegram_audio_sync(user_id, audio_bytes)
    except requests.RequestException as e:
        glados_mqtt.debug(f"Error al generar respuesta de audio: {e}")

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
        glados_mqtt.debug(f"Error al enviar el archivo de audio a la API: {e}")


# Iniciar cliente de Telegram
telegram_client = TelegramClient('/data/tg_sess.ignore', api_id, api_hash)

# Función para enviar mensajes de Telegram de forma síncrona
def send_telegram_message_sync(user_id, message):
    asyncio.run_coroutine_threadsafe(telegram_client.send_message(user_id, message), loop)

def send_telegram_audio_sync(user_id, audio_bytes):
    asyncio.run_coroutine_threadsafe(telegram_client.send_file(user_id, audio_bytes, voice_note=True), loop)

def edit_telegram_message_sync(chat_id, message_id, new_text):
    asyncio.run_coroutine_threadsafe(telegram_client.edit_message(chat_id,message_id, new_text), loop)

async def edit_telegram_message(chat_id, message_id, new_text):
    await telegram_client.edit_message(chat_id, message_id, new_text)

# Evento para manejar nuevos mensajes en Telegram
@telegram_client.on(events.NewMessage)
async def handle_new_message(event):
    glados_mqtt.debug(event.stringify())

    # Enviar respuesta inicial "Procesando..."
    response = await telegram_client.send_message(event.chat_id, "Procesando...")
    # Intenta obtener el nombre completo del usuario
    sender_name = "Desconocido"
    if event.sender:
        # Si el evento tiene un enviador, construye el nombre completo si es posible
        sender_name = event.sender.first_name or ""
        if event.sender.last_name:
            sender_name += f" {event.sender.last_name}"

    # Procesamiento de mensajes de voz o audio
    if event.message.voice or event.message.audio:
        # Descargar el archivo de audio
        audio_file_path = await telegram_client.download_media(event.message, file='/tmp/audio/')
        await edit_telegram_message(event.chat_id,response.id, "Whisper...")
        # Procesar el archivo de audio
        #mas adelante pensar en agregar un campo para especificar que es una transcripcion para poder pasarle esa info al LLM
        transcription = await send_audio_to_transcription_api(audio_file_path)
        event_data = {
            'message_text': transcription,
            'sender_id': event.sender_id,
            'channel_id': event.chat_id,
            'reply_msg_id' : response.id,
            'sender_name': sender_name
            # Agrega aquí otros campos relevantes
        }
        await edit_telegram_message(event.chat_id,response.id, "LLM...")
        glados_mqtt.publish(topic_telegram_event, json.dumps(event_data))
        return

    # Procesamiento de mensajes de texto
    if event.is_private and event.sender_id != 771352834:  # Asegúrate de actualizar este ID según sea necesario
        event_data = {
            'message_text': event.message.message if event.message else None,
            'sender_id': event.sender_id,
            'channel_id': event.chat_id,
            'reply_msg_id' : response.id,
            'sender_name': sender_name
            # Agrega aquí otros campos relevantes
        }
        await edit_telegram_message(event.chat_id,response.id, "LLM...")
        glados_mqtt.publish(topic_telegram_event, json.dumps(event_data))



# Iniciar la instancia de GladosMQTT
glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName, msg_callback=on_mqtt_message)
glados_mqtt.set_topics([topic_telegram_send_msg,topic_telegram_edit_msg])


if __name__ == "__main__":
    glados_mqtt.init_mqtt()
    telegram_client.start(bot_token=telegram_token)
    loop = asyncio.get_event_loop()
    loop.run_until_complete(telegram_client.run_until_disconnected())