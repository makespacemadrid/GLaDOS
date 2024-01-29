# -*- coding: utf-8 -*-

#Nodo Mqtt de ejemplo, 
#gladosMQTT.py contiene la funcionalidad para manejar el mqtt.
# aui nos conectamos y reaccionamos a los mensajes que llegan.
# TODO Documentar mejor...  gpt? xD

import os
import gladosMQTT
import platform
import time


#Variables
mqHost	 = os.environ.get("MQTT_HOST")
mqPort 	 = os.environ.get("MQTT_PORT")
nodeName = platform.node()


def subscribeTopics() :
	gladosMQTT.subscribe("node/topic")

def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_message(client, userdata, msg):
	if (msg.topic == "my_topic") :
		gladosMQTT.debug("cmd:"+msg)
	
def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

gladosMQTT.initMQTT(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)

try:
	while True:
		#Loop principal del programa
		time.sleep(10)
except KeyboardInterrupt:
	print('interrupted!')
