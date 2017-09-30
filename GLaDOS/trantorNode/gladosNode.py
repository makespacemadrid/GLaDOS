import paho.mqtt.client as mqtt
import time
import os
import platform
import psutil
import subprocess
from threading import Timer
from generateSpaceApiJson import generateSpaceApi


mqttServer = "localhost"
mqttPort   = 1883
nodeName   = "server"

globalShutdown = False

updateNomi     = "False"
updateSpaceApi = "False"

commandOpen 			= "echo Aqui el comando para abrir!"
commandClose 			= "echo Aqui el comando para cerrar!"
commandUpdateSpaceApi 	= "echo Aqui el comando para actualizar el json"


coffe_made   = 100


def openSpace():
		global coffe_made
		global updateNomi
		global updateSpaceApi
		
		print("Espacio abierto")
		generateSpaceApi(True,coffe_made)	
		
		print("UPDaTE: "+updateNomi+" "+updateSpaceApi)
		
		if updateNomi == "true":
			return_code = subprocess.call(commandOpen,shell=True)
		if updateSpaceApi == "true" :
			return_code = subprocess.call(commandUpdateSpaceApi,shell=True)
	
def closeSpace():
		global coffe_made
		global updateNomi
		global updateSpaceApi
		print("Espacio cerrado")
		generateSpaceApi(False,coffe_made)	
		print("UPDaTE: "+updateNomi+" "+updateSpaceApi)
		if updateNomi == "true" :
			return_code = subprocess.call(commandClose,shell=True)
		if updateSpaceApi == "true" :
			return_code = subprocess.call(commandUpdateSpaceApi,shell=True)


def getSystemInfo():
	procType = str(platform.machine())
	osType   = str(platform.system())
	print("Architecture : "+procType+" OS: "+osType)

def publishSystemInfo():
	mqttClient.publish("node/"+nodeName+"/system/cpu", platform.machine())
	mqttClient.publish("node/"+nodeName+"/system/os",platform.system())	

def publishSystemStats():
	try:
		mqttClient.publish("node/"+nodeName+"/system/cpuUsage", psutil.cpu_percent(interval=1))
	except:
		pass
	try:
		mqttClient.publish("node/"+nodeName+"/system/temperatures", str(psutil.sensors_temperatures()))
	except:
		pass
	try:	
		mqttClient.publish("node/"+nodeName+"/system/diskUsage", str(psutil.disk_usage('/')[3]))
	except:
		pass


def powerOffSystem():
	mqttClient.publish("node/"+nodeName+"/system/status", "poweroff")
	print("Shutting down (M$ style)!")
	os.system('shutdown -s') # Windows, intenta las dos maneras de apagar... y que funcione la que corresponda.
	print("Shutting down (Linux style)!")
	os.system('systemctl poweroff -i') #Linux

def subscribeTopics():
	mqttClient.subscribe("space/powerStatus")
	mqttClient.subscribe("space/status")
	mqttClient.subscribe("space/reportStatusNomi")
	mqttClient.subscribe("space/reportStatusSpaceApi")
	
def on_connect(client, userdata, rc,arg):
	print("Connected with result code "+str(rc))
	mqttClient.publish("node/"+nodeName+"/system/status", "connected")
	subscribeTopics()
	publishSystemInfo()

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	global updateNomi
	global updateSpaceApi
		
	print("RCV: "+msg.topic + " - " +msg.payload)
	
	if (msg.topic == "space/status") :
		if 	msg.payload == "Open" :
			openSpace()
		elif msg.payload == "Close" :
			closeSpace()

	elif (msg.topic == "space/powerStatus") and globalShutdown :
		if msg.payload == "off" :
			powerOffSystem()

	elif (msg.topic == "space/reportStatusNomi"):
		updateNomi = msg.payload
	elif msg.topic == "space/reportStatusSpaceApi":
		updateSpaceApi = msg.payload


def on_disconnect(client, userdata, rc):
	print("Disconnected! rc: "+str(rc))


print("Init...")
print("[GladosNode] Connecting : "+mqttServer+" Port:"+str(mqttPort))
print("[GladosNode] Global Shutdown : "+ str(globalShutdown))
getSystemInfo()
print("-----------------------------------------------------------")


import pystray
icon = pystray.Icon('test name')
from PIL import Image, ImageDraw
# Generate an image
width = 64
height = 64
image = Image.new('RGB', (width, height),(125,125,0))
dc = ImageDraw.Draw(image)
dc.rectangle((width // 2, 0, width, height // 2),(125,0,0))
dc.rectangle((0, height // 2, width // 2, height),(125,0,0))
icon.image = image
icon.icon = image
icon.visible = True

mqttClient = mqtt.Client()
mqttClient.on_connect    = on_connect
mqttClient.on_message    = on_message
mqttClient.on_disconnect = on_disconnect

#client.will_set('/outbox/'+clientName+'/lwt', 'anythinghere', 0, False)

mqttClient.loop_start()

try:
	mqttClient.connect(mqttServer, mqttPort, 60)
except:
	print("Cant connect, will retry automatically")


while True : 
	time.sleep(10)
	publishSystemStats()
#	icon.run()
