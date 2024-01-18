#!/usr/bin/env python
# -*- coding: utf-8 -*-

# pip install paho-mqtt psutil

import time
import platform
import gladosMQTT
import telegramBot


#Variables
mqHost	 = "10.0.0.10"
mqPort 	 = 1883
nodeName = platform.node()
globalCMDTopic = "space/cmnd"

botToken	  = '808661769:AAG1TasaPfGRwzqaKVdux8AXI1AM9L2k0KI'
botMasterUser = 42489438
userList	  = []
adminList	  = []

helpText	  = "\
Comandos disponibles:\n \
/help"


def subscribeTopics() :
	gladosMQTT.subscribe("node")

def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_message(client, userdata, msg):
	if (msg.topic == commandTopic) :
		debug("cmd:"+msg)
	
def on_disconnect(client, userdata, rc):
	debug("Disconnected! rc: "+str(rc))


gladosMQTT.initMQTT(mqHost , mqPort , nodeName , on_connect , on_message , on_disconnect , globalCMDTopic)

telegramBot.initBot(botToken , botMasterUser , userList , adminList , helpText)

try:
	while True:
		#Loop principal del programa
		time.sleep(10)
except KeyboardInterrupt:
	print('interrupted!')
