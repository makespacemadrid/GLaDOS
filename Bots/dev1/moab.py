import os
import threading

import GladosIA
import GladosTelegramClient
import GladosDiscordClient
import GladosSlackClient
import GladosMQTT

import platform
import GladosMQTT
import time

# Variables
mqHost = "10.0.0.20"
mqPort = 1883
nodeName = platform.node()
globalCMDTopic = "space/cmnd"

glados_bot = GladosIA.GladosBot()
discord_bot = GladosDiscordClient.GladosDiscordClient()


def subscribeTopics():
    gladosMQTT.subscribe("node")


def on_connect(client, userdata, rc, arg):
    subscribeTopics()


def on_message(client, userdata, msg):
    if (msg.topic == commandTopic):
        debug("cmd:"+msg)


def on_disconnect(client, userdata, rc):
    debug("Disconnected! rc: "+str(rc))


def run_bots():
    global mqHost, mqPort, nodeName, on_connect, on_message, on_disconnect, globalCMDTopic
    GladosMQTT.initMQTT(mqHost, mqPort, nodeName, on_connect,
                        on_message, on_disconnect, globalCMDTopic)

#  GladosDiscordClient.run_bot(GladosIA.askGLaDOS)
#  GladosSlackClient.run_bot(GladosIA.askGLaDOS)
#  tg_t = threading.Thread(target=	GladosTelegramClient.run_bot, args=(GladosIA.askGLaDOS,))
    dc_t = threading.Thread(target=discord_bot.run, args=())
#  sl_t = threading.Thread(target=	GladosSlackClient.run_bot   , args=(GladosIA.askGLaDOS,))

#  tg_t.start()
    dc_t.start()
#  sl_t.start()

#  tg_t.join()
#  dc_t.join()


async def process_msgs():
    pending = discord_bot.getMsgs()
    for msg in pending:
        response = glados_bot.askGLaDOS(msg)
        await discord_bot.sendMsg(response)
