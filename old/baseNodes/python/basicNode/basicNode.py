#
# Ejemplo mínimo de conexión mqtt con python
#
#
#
#-Requerimientos:
#	sudo apt install python-pip
#	pip install paho-mqtt

import paho.mqtt.client as mqtt

#Variables
mqttServer = "10.0.0.10"
mqttPort   = 1883


def on_connect(client, userdata, rc,arg):
	print("Connected with result code "+str(rc))
	#mqttClient.subscribe("topic/topic")
	#mqttClient.publish("node/hello","Hello world!",false)


# Esta funcion se ejecuta cada vez que llega un mensaje, el topico esta en msg.topic y el contenido del mensaje en msg.payload
def on_message(client, userdata, msg):
	print("RCV: "+msg.topic + " - " +msg.payload)
#	if (msg.topic == "space/powerStatus") :
#		if msg.payload == "off" :
#			powerOffSystem()

def on_disconnect(client, userdata, rc):
	print("Disconnected! rc: "+str(rc))



#Inicio del programa

print("Connecting : "+mqttServer+" Port:"+str(mqttPort))

#conexion mqtt
mqttClient 				 = mqtt.Client()
mqttClient.on_connect    = on_connect
mqttClient.on_message    = on_message
mqttClient.on_disconnect = on_disconnect
#client.will_set('/outbox/'+clientName+'/lwt', 'anythinghere', 0, False)

try:
	mqttClient.connect(mqttServer, mqttPort, 60)
except:
	print("Cant connect, will retry automatically")

mqttClient.loop_start()

#loop del programa
while True :
	time.sleep(10)
