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
mqHost	 = os.environ.get("MQTT_HOST", "mqtt.makespacemadrid.org")
mqPort 	 = os.environ.get("MQTT_PORT", 1883)
nodeName = platform.node()


def subscribeTopics() :
	gladosMQTT.subscribe("space/status")

def on_connect(client, userdata, rc,arg):
	subscribeTopics()

def on_message(client, userdata, msg):
	if (msg.topic == "space/status") :
		# Extraer la carga útil y decodificarla a una cadena de texto
		payload = msg.payload.decode('utf-8')
		# Guardar el payload en un archivo
		with open('/spaceapi/status.json', 'w') as file:
			file.write(payload)
		print("Recibido:", payload)  # Imprimir el payload para depuración
		# Intentar parsear el JSON
		try:
			data = json.loads(payload)
			open_status = data['state']['open']
			gladosMQTT.debug("open: "+open_status)
		except json.JSONDecodeError as e:
			print("Error al parsear JSON:", e)

	
def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

gladosMQTT.initMQTT(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)

try:
	while True:
		#Loop principal del programa
		time.sleep(10)
		print("ping!")
except KeyboardInterrupt:
	print('interrupted!')
