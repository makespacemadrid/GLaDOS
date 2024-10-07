# -*- coding: utf-8 -*-
import os
import json
import platform
import discord
from discord.ext import commands
from gladosMQTT import GladosMQTT
import asyncio

# Configuración de Discord
discord_token = os.environ.get("DISCORD_BOT_TOKEN")
discordbot_prefix = "!"
discordintents = discord.Intents.default()
discordintents.messages = True
discordintents.members = True
discordintents.message_content = True
bot = commands.Bot(command_prefix=discordbot_prefix, intents=discordintents)

# Configuración MQTT
mqHost = os.environ.get("MQTT_HOST")
mqPort = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()

# Temas MQTT
topic_discord = "comms/discord"
topic_discord_send_msg = topic_discord + "/send_id"
topic_discord_edit_msg = topic_discord + "/edit_msg"
topic_discord_event = topic_discord + "/event"

# Instancia de GladosMQTT
glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName)
glados_mqtt.set_topics([topic_discord_send_msg,topic_discord_edit_msg])

# Función para manejar mensajes MQTT
def on_mqtt_message(client, userdata, msg):
    if msg.topic == topic_discord_send_msg:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            sendDiscordMsg_sync(data['dest'], data['msg'])
        except json.JSONDecodeError as e:
            glados_mqtt.debug("Error al parsear JSON:", e)

# Configurar callbacks MQTT
glados_mqtt.mqttClient.on_message = on_mqtt_message


def sendDiscordMsg_sync(channel_id, msg):
    glados_mqtt.debug(f'Enviando a {channel_id} {msg}')

    channel = bot.get_channel(channel_id)
    if channel:
        asyncio.run_coroutine_threadsafe(channel.send(msg), loop)
    else:
        glados_mqtt.debug(f'Error, no se ha encontrado el canal {channel_id}')

# Función para enviar mensajes a Discord
async def sendDiscordMsg(channel_id, msg):
    glados_mqtt.debug(f'Enviando a {channel_id} {msg}')

    channel = bot.get_channel(channel_id)
    if channel:
        await channel.send(msg)
    else:
        glados_mqtt.debug(f'Error, no se ha encontrado el canal {channel_id}')

# Eventos de Discord
@bot.event
async def on_ready():
    print(f'Bot conectado como {bot.user.name} - {bot.user.id}')

@bot.event
async def on_message(message):
    if message.author == bot.user:
        return
    message_data = {
        'content': str(message.content),
        'author_id': message.author.id,
        'author_name': str(message.author),
        'channel_id': message.channel.id,
        'channel_name': str(message.channel),
        'mentions': str(message.mentions)
    }
    #await message.channel.send('Recibí tu mensaje: ' + message.content)  # Respuesta de ejemplo
    glados_mqtt.publish(topic_discord_event, json.dumps(message_data))

if __name__ == "__main__":
    glados_mqtt.init_mqtt()
    loop = asyncio.get_event_loop()
    loop.run_until_complete(bot.run(discord_token))
