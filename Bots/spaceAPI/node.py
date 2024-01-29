# -*- coding: utf-8 -*-

#Nodo Mqtt de ejemplo, 
#gladosMQTT.py contiene la funcionalidad para manejar el mqtt.
# aui nos conectamos y reaccionamos a los mensajes que llegan.
# TODO Documentar mejor...  gpt? xD

import os

import gladosMQTT
import platform
import time
import json

#Variables
mqHost	 = str(os.environ.get("MQTT_HOST"))
mqPort 	 = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()


def subscribeTopics() :
	gladosMQTT.subscribe("space/status")

def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_message(client, userdata, msg):
	if (msg.topic == "space/status") :
		try:
			# Extraer la carga útil y decodificarla a una cadena de texto
			payload = msg.payload.decode('utf-8')
			data = json.loads(payload)
			open_status = data['state']['open']
			if open_status:
				gladosMQTT.debug("open!")
			else:
				gladosMQTT.debug("closed!")
			with open('/spaceapi/status.json', 'w') as file:
				file.write(payload)
			print("Recibido:", payload)  # Imprimir el payload para depuración
			
		except json.JSONDecodeError as e:
			print("Error al parsear JSON:", e)

	
def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

gladosMQTT.initMQTTandLoopForever(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)