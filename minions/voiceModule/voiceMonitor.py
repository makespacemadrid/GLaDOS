import subprocess
import paho.mqtt.client as mqtt

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc,arg):
	print("Connected with result code "+str(rc))
	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	client.subscribe("space/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	if str(msg.payload) == "Open" :
		return_code = subprocess.call("ruby gladosTTS.rb \"Espacio Abierto!\"",shell=True)
	elif str(msg.payload) == "Close":
		return_code = subprocess.call("ruby gladosTTS.rb \"Espacio Cerrado!\"",shell=True) 
	elif str(msg.payload) == "on":
		return_code = subprocess.call("ruby gladosTTS.rb \"Encendiendo! \"",shell=True) 
	elif str(msg.payload) == "off":
		return_code = subprocess.call("ruby gladosTTS.rb \"Apagando! \"",shell=True)
	elif str(msg.payload) == "powerOffRequest":
		return_code = subprocess.call("ruby gladosTTS.rb \"ATENCION, Apagando en 10 segundos!\"",shell=True) 				
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("trantor", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
