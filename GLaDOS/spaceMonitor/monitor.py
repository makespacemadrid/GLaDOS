import subprocess
import paho.mqtt.client as mqtt
from generateSpaceApiJson import generateSpaceApi

coffe_made   = 100
commandOpen  = "echo Aqui el comando para abrir!"
commandClose = "echo Aqui el comando para cerrar!"


def openSpace():
		global coffe_made
		print("Espacio abierto")
		generateSpaceApi(True,coffe_made)	
		return_code = subprocess.call(commandOpen,shell=True)
	
def closeSpace():
		global coffe_made
		print("Espacio cerrado")
		generateSpaceApi(False,coffe_made)	
		return_code = subprocess.call(commandClose,shell=True)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc,arg):
	print("Connected with result code "+str(rc))
	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	client.subscribe("space/status")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	print("RCV:"+msg.payload)
	if str(msg.payload) == "Open" :
		openSpace()
	elif str(msg.payload) == "Close":
		closeSpace()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("localhost", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
