import paho.mqtt.client as mqtt
import time
import os
import platform
import psutil
from threading import Timer
import platform



#Variables

mqttServer = "10.0.0.10"
mqttPort   = 1883

nodeName   = platform.node()
globalShutdown = False


#Funciones, el programa es mas abajo

def getSystemInfo():
	procType = str(platform.machine())
	osType   = str(platform.system())
	print("Hostname :" + nodeName +", Architecture : "+procType+" OS: "+osType)

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
	mqttClient.publish("node/"+nodeName+"/status", "poweroff")
	print("Shutting down (M$ style)!")
	os.system('shutdown -s') # Windows, intenta las dos maneras de apagar... y que funcione la que corresponda.
	print("Shutting down (Linux style)!")
	os.system('systemctl poweroff -i') #Linux

def debug(msg) :
	mqttClient.publish("node/"+nodeName+"/debug",msg)


def on_connect(client, userdata, rc,arg):
	print("Connected with result code "+str(rc))
	mqttClient.subscribe("space/powerStatus")
	mqttClient.subscribe("node/"+nodeName+"/cmnd")
	procType = str(platform.machine())
	osType   = str(platform.system())
	mqttClient.publish("node/hello", "Hello! Im "+ nodeName +", Architecture : "+procType+" OS: "+osType)
	publishSystemInfo()

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	print("RCV: "+msg.topic + " - " +msg.payload)
	
	if (msg.topic == "space/powerStatus") and globalShutdown :
		if msg.payload == "off" :
			powerOffSystem()
	elif (msg.topic == "node/"+nodeName+"/cmnd") :
		if msg.payload == "poweroff":
			powerOffSystem()

def on_disconnect(client, userdata, rc):
	print("Disconnected! rc: "+str(rc))



#Inicio del programa

print("Init...")
print("[GladosNode] Connecting : "+mqttServer+" Port:"+str(mqttPort))
print("[GladosNode] Global Shutdown : "+ str(globalShutdown))
getSystemInfo()
print("-----------------------------------------------------------")


#Icono para el tray del sistema
#import pystray
#icon = pystray.Icon('test name')
#from PIL import Image, ImageDraw
#width = 64
#height = 64
#image = Image.new('RGB', (width, height),(125,125,0))
#dc = ImageDraw.Draw(image)
#dc.rectangle((width // 2, 0, width, height // 2),(125,0,0))
#dc.rectangle((0, height // 2, width // 2, height),(125,0,0))
#icon.image = image
#icon.icon = image
#icon.visible = True

#conexion mqtt
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

#loop del programa
while True : 
	time.sleep(10)
	publishSystemStats()
#	icon.run()
