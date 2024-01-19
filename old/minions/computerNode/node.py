#!/usr/bin/env python
# -*- coding: utf-8 -*-

# pip install paho-mqtt psutil

import platform
import gladosMQTT
import time

#Variables
mqHost	 = "10.0.0.10"
mqPort 	 = 1883
nodeName = platform.node()
globalCMDTopic = "space/cmnd"


def subscribeTopics() :
	gladosMQTT.subscribe("node")

def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_message(client, userdata, msg):
	if (msg.topic == commandTopic) :
		debug("cmd:"+msg)
	
def on_disconnect(client, userdata, rc):
	debug("Disconnected! rc: "+str(rc))


gladosMQTT.initMQTT(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect,globalCMDTopic)


try:
	while True:
		#Loop principal del programa
		time.sleep(10)
except KeyboardInterrupt:
	print('interrupted!')
