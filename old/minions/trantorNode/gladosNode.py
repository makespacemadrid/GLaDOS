import paho.mqtt.client as mqtt
import time
import os
import platform
import psutil
import subprocess
import urllib
from privateCommands import *
from threading import Timer
from generateSpaceApiJson import generateSpaceApi


mqttServer = "10.0.0.10"
mqttPort   = 1883
nodeName   = "server"

nomiEnabled     = "false"
spaceApiEnabled = "false"
coffe_made      = 666
temperature     = 42
humidity        = 42
spaceOpen       = False
spaceLocked     = False

def updateSpaceApi():
	global spaceApiEnabled
	global temperature
	global humidity
	global coffe_made
	global spaceOpen
	if spaceApiEnabled == "true" :
		generateSpaceApi(spaceOpen,temperature,humidity,coffe_made)
		return_code = subprocess.call(commandUpdateSpaceApi,shell=True)	

def openSpace():
	global nomiEnabled
	global spaceOpen
	print("Espacio abierto")
	spaceOpen = True
	if nomiEnabled == "true":
		return_code = subprocess.call(commandOpen,shell=True)
	updateSpaceApi()

def closeSpace():
	global nomiEnabled
	global spaceOpen
	print("Espacio cerrado")
	spaceOpen = False
	if nomiEnabled == "true" :
		return_code = subprocess.call(commandClose,shell=True)
	updateSpaceApi()

def announceNomi(msg):
	print("announce nomi: "+msg)
	if nomiEnabled == "true" :
		cmd = commandNomiAnnounce+urllib.quote(msg)
		return_code = subprocess.call(cmd,shell=True)


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
		mem = psutil.virtual_memory().available
		print(mem)
		mqttClient.publish("node/"+nodeName+"/system/availableMemmory", str(mem))
	except:
		pass

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
	try:
		result = subprocess.check_output("docker ps | wc -l", shell=True)
		mqttClient.publish("node/"+nodeName+"/system/dockerContainersRunning", int(result))
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
	mqttClient.subscribe("space/Nomi/announce")
	mqttClient.subscribe("space/Nomi/enabled")
	mqttClient.subscribe("space/spaceApi/enabled") 
	mqttClient.subscribe("space/spaceApi/update") 
	mqttClient.subscribe("space/coffeeMade")
	mqttClient.subscribe("space/temperature")
	mqttClient.subscribe("space/humidity")
	mqttClient.subscribe("space/locked")

def on_connect(client, userdata, rc,arg):
	print("Connected with result code "+str(rc))
	mqttClient.publish("node/"+nodeName+"/system/status", "connected")
	subscribeTopics()
	publishSystemInfo()

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	global nomiEnabled
	global spaceApiEnabled
	global coffe_made
	global temperature
	global humidity
	global spaceLocked

	print("RCV: "+msg.topic + " - " +msg.payload)
	if (msg.topic == "space/status") :
		if 	msg.payload == "Open" :
			openSpace()
		elif msg.payload == "Close" :
			closeSpace()

	elif (msg.topic == "space/Nomi/enabled"):
		nomiEnabled = msg.payload
        elif (msg.topic == "space/Nomi/announce"):
		announceNomi(msg.payload);
	elif msg.topic == "space/spaceApi/enabled":
		spaceApiEnabled = msg.payload
	elif msg.topic == "space/spaceApi/update":
		updateSpaceApi()
	elif msg.topic == "space/coffeMade":
		coffe_made = msg.payload
	elif msg.topic == "space/temperature":
		temperature = msg.payload
	elif msg.topic == "space/humidity":
		humidity = msg.payload
	elif msg.topic == "space/locked":
		newStatus = (msg.payload == "locked")
		if (newStatus != spaceLocked) :
			spaceLocked = newStatus;
			if (spaceLocked == True) :
				announceNomi("Cerrojos cerrados!")
			else :
				announceNomi("Cerrojos abiertos!")

def on_disconnect(client, userdata, rc):
	print("Disconnected! rc: "+str(rc))


print("Init...")
print("[GladosNode] Connecting : "+mqttServer+" Port:"+str(mqttPort))
getSystemInfo()
print("-----------------------------------------------------------")


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
