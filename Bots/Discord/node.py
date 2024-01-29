# -*- coding: utf-8 -*-
import os
import json
import gladosMQTT
import platform
import os
import discord
from discord.ext import commands


discord_token = str(os.environ.get("DISCORD_BOT_TOKEN"))  # Reemplaza con tu token de bot
discordbot_prefix = "!"

discordintents = discord.Intents.default()
discordintents.messages = True
discordintents.members = True
discordintents.message_content = True
bot = commands.Bot(
	command_prefix=discordbot_prefix, intents=discordintents)


# Configuración MQTT
#Variables
mqHost	 = str(os.environ.get("MQTT_HOST"))
mqPort 	 = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()


# Temas MQTT para comunicarse con otros componentes
topic_discord = "comms/discord"
topic_discord_send_msg = topic_discord + "/send"
topic_discord_event = topic_discord + "/event"


discord_client = commands.Bot(command_prefix=discordbot_prefix, intents=discordintents)


def subscribeTopics() :
	gladosMQTT.subscribe(topic_discord_send_msg)


def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

def on_message(client, userdata, msg):
	if (msg.topic == topic_discord_send_msg) :
		try:
			payload = msg.payload.decode('utf-8')
			data = json.loads(payload)
			dest    = data['dest']
			content = data['msg']
			sendDiscordMsg(dest,content)
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")



# Enviar mensajes a Discord
async def sendDiscordMsg(channel_id, msg):
    channel = discord_client.get_channel(channel_id)
    if channel:
        await channel.send(msg)


@discord_client.event
async def on_ready():
	print(f'Bot conectado como {discord_client.user.name} - {discord_client.user.id}')


# Evento para manejar nuevos mensajes en Discord
@discord_client.event
async def on_message(message):
	if message.author == discord_client.user:
		return
	# Crear un diccionario con la información relevante
	message_data = {
		'content': message.content,
		'author_id': message.author.id,
		'author_name': str(message.author),
		'channel_id': message.channel.id,
		'channel_name': str(message.channel),
		'mentions': str(message.mentions)
		# Puedes agregar aquí cualquier otro dato que necesites
	}

	# Convertir el diccionario a JSON y publicarlo
	gladosMQTT.publish(topic_discord_event, json.dumps(message_data))

	await message.channel.send('Recibí tu mensaje: ' + message.content)  # Respuesta de ejemplo

# Iniciar el bot de Discord
def start_discord_bot():
    discord_client.run(discord_token)

    
if __name__ == "__main__":
    gladosMQTT.initMQTT(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)
    start_discord_bot()
