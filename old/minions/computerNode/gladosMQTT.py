#!/usr/bin/env python
# -*- coding: utf-8 -*-

import paho.mqtt.client as mqtt
import time
import os
import platform
import psutil
from threading import Timer
import json



#Carga inicial de variables, aunque el valor de estas se reescribira al ejecutar initMQTT
mqttServer			= "192.168.1.1"
mqttPort			= 1883
nodeName			= "node"
baseTopic			= "/node/"
debugTopic			= "debug"
commandTopic		= "cmnd"
globalCommandTopic	= ""
globalShutdown 		= False
mqttClient 	   		= mqtt.Client()

def dummy() :
	debug(dummy)

nodeConnectedCallback 		= dummy
nodeMsgCallback				= dummy
nodeDisconnectedCallback	= dummy

def subscribe(topic) :
	mqttClient.subscribe(topic)
	
def subscribeTopics() :
	subscribe(globalCommandTopic)
	subscribe(commandTopic)
	
def publish(topic,msg,persist = False) :
	mqttClient.publish(topic,msg,persist)

def debug(msg) :
	print(msg)
	publish(debugTopic,msg)


def on_connect(client, userdata, rc,arg):
	debug("[GladosNode] Connected with result code "+str(rc))
	debug("[GladosNode] Node name      : " + nodeName +" ,Global Shutdown : "+ str(globalShutdown))
	debug("[GladosNode] Base topic     : " + baseTopic)
	debug("[GladosNode] Command topic  : " + commandTopic)
	debug("[GladosNode] Global command : " + globalCommandTopic)	
	debug("[GladosNode] Debug topic    : " + debugTopic)
	debug("[GladosNode] mosquitto_sub -h " + mqttServer + " -t " + debugTopic)
	
	procType = str(platform.machine())
	osType   = str(platform.system())
	publish("node/hello", "Hello! Im "+ nodeName +", Architecture : "+procType+" OS: "+osType)
	subscribeTopics()
	publishSystemInfo()
	nodeConnectedCallback(client, userdata, rc,arg)

def processGlobalCMD(cmd) :
	if   cmd == "poweroff" : powerOffSystem()
	else : return False
	return True

def processCMD(cmd) :
	if   cmd == "poweroff" : powerOffSystem()
	else : return False
	return True

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
	debug("[GladosNode] mqtt_rcv: {  "+msg.topic + " - " +msg.payload+ "  }")
	
	if (msg.topic == globalCommandTopic) :
		if not processGlobalCMD(msg.payload) :
			nodeMsgCallback(client, userdata, msg)
	elif (msg.topic == commandTopic) :
		if not processCMD(msg.payload) :
			nodeMsgCallback(client, userdata, msg)
	else :
		nodeMsgCallback(client, userdata, msg)

def on_disconnect(client, userdata, rc):
	nodeDisconnectedCallback(client, userdata, rc)


def initMQTT(host,port,name,connectedCallback,msgCallback,disconnectedCallback,globalCMDTopic = 'cmnd') :
	global mqttServer
	global mqttPort
	global nodeName
	global baseTopic
	global debugTopic
	global commandTopic
	global globalCommandTopic
	global mqttClient

	mqttServer			= host
	mqttPort			= port
	nodeName			= name
	baseTopic			= "/node/"+nodeName+"/"
	debugTopic			= baseTopic+"debug"
	commandTopic		= baseTopic+"cmnd"
	globalCommandTopic	= globalCMDTopic

	debug("[GladosNode] Connecting : "+mqttServer+" Port:"+str(mqttPort))

	nodeConnectedCallback 		= connectedCallback
	nodeMsgCallback				= msgCallback
	nodeDisconnectedCallback	= disconnectedCallback

	mqttClient.on_connect    	= on_connect
	mqttClient.on_message    	= on_message
	mqttClient.on_disconnect 	= on_disconnect

	try:
		mqttClient.connect(mqttServer, mqttPort, 60)
	except:
		print("Cant connect, will retry automatically")
	mqttClient.loop_start()

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
	publish(baseNode+"/status", "poweroff")
	if(platform.system() == "Linux"):
		debug("Shutting down (Linux style)!")
		os.system('sudo systemctl poweroff -i') #Linux
	else:
		debug("Shutting down (M$ style)!")
		os.system('shutdown -s') # Windows
