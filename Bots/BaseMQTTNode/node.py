# -*- coding: utf-8 -*-

import os
import gladosMQTT
import platform
import time


#Variables
mqHost	 = os.environ.get("MQTT_HOST", "10.0.10.10")
mqPort 	 = os.environ.get("MQTT_PORT", 1883)
nodeName = os.environ.get("NODE_NAME", platform.node())


def subscribeTopics() :
	gladosMQTT.subscribe("node/topic")

def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_message(client, userdata, msg):
	if (msg.topic == "my_topic") :
		gladosMQTT.debug("cmd:"+msg)
	
def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

print("start!")



try:
	while True:
		#Loop principal del programa
		time.sleep(10)
		print("ping!")
except KeyboardInterrupt:
	print('interrupted!')
