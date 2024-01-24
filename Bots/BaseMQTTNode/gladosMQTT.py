# -*- coding: utf-8 -*-

import paho.mqtt.client as mqtt
import platform
import time
import os
import json



#Carga inicial de variables, aunque el valor de estas se reescribira al ejecutar initMQTT
mqttServer			= "192.168.1.1"
mqttPort			= 1883
nodeName			= "test-node"
baseTopic			= "node/test-node"
debugTopic			= baseTopic+"debug"
mqttClient 	   		= mqtt.Client()


def dummy() :
	debug(dummy)

nodeConnectedCallback 		= dummy
nodeMsgCallback				= dummy
nodeDisconnectedCallback	= dummy

def subscribe(topic) :
	mqttClient.subscribe(topic)
	
	
def publish(topic,msg,persist = False) :
	mqttClient.publish(topic,msg,persist)

def debug(msg):
    # Convertir el mensaje a una cadena, si es necesario
    if not isinstance(msg, str):
        try:
            # Intenta convertir el mensaje a una cadena JSON si es un objeto complejo
            msg = json.dumps(msg)
        except (TypeError, ValueError):
            # Si la conversión falla, usa una representación de string genérica
            msg = str(msg)
    
    print(msg)
    publish(debugTopic, msg)

def on_connect(client, userdata, rc,arg):
	global mqttServer
	global mqttPort
	global nodeName
	global baseTopic
	global debugTopic

	debug("[GladosNode] Connected with result code "+str(rc))
	debug("[GladosNode] Node name      : " + nodeName)
	debug("[GladosNode] Base topic     : " + baseTopic)
	debug("[GladosNode] Debug topic    : " + debugTopic)
	debug("[GladosNode] mosquitto_sub -h " + mqttServer + " -t " + debugTopic)


	publish("node/hello", "Hello! Im "+ nodeName)
	nodeConnectedCallback(client, userdata, rc,arg)


# The callback for when a PUBLISH message is received from the server.
# Aqui recibimos los mensajes, y si no hay que hacer nada con ellos se lo pasamos al callback del nodo para que los procese 
def on_message(client, userdata, msg):

	try:
		debug("[GladosNode] mqtt_rcv: {  "+msg.topic + " - " +msg.payload+ "  }")
	except:	
		debug("[GladosNode] mqtt_rcv: not str ")
	nodeMsgCallback(client, userdata, msg)

def on_disconnect(client, userdata, rc):
	nodeDisconnectedCallback(client, userdata, rc)

#Inicia mqtt y devuelve el control
def initMQTT(host,port,name,connectedCallback,msgCallback,disconnectedCallback) :
	global mqttServer
	global mqttPort
	global nodeName
	global baseTopic
	global debugTopic
	global mqttClient

	global nodeConnectedCallback
	global nodeMsgCallback
	global nodeDisconnectedCallback

	mqttServer			= host
	mqttPort			= port
	nodeName			= name
	baseTopic			= "node/"+nodeName+"/"
	debugTopic			= baseTopic+"debug"


	nodeConnectedCallback 		= connectedCallback
	nodeMsgCallback				= msgCallback
	nodeDisconnectedCallback	= disconnectedCallback

	mqttClient.on_connect    	= on_connect
	mqttClient.on_message    	= on_message
	mqttClient.on_disconnect 	= on_disconnect

	print("[GladosNode] Connecting : "+str(mqttServer)+" Port:"+str(mqttPort)+ " node: " + str(name))

	try:
		mqttClient.connect(mqttServer,port=mqttPort,keepalive=120)
	except:
		print("Cant connect, will retry automatically")
	mqttClient.loop_start()

#Inicia mqtt y captura la ejecucion
def initMQTTandLoopForever(host,port,name,connectedCallback,msgCallback,disconnectedCallback) :
	global mqttServer
	global mqttPort
	global nodeName
	global baseTopic
	global debugTopic

	global nodeConnectedCallback
	global nodeMsgCallback
	global nodeDisconnectedCallback

	mqttServer			= host
	mqttPort			= port
	nodeName			= name
	baseTopic			= "node/"+nodeName+"/"
	debugTopic			= baseTopic+"debug"


	nodeConnectedCallback 		= connectedCallback
	nodeMsgCallback				= msgCallback
	nodeDisconnectedCallback	= disconnectedCallback

	mqttClient.on_connect    	= on_connect
	mqttClient.on_message    	= on_message
	mqttClient.on_disconnect 	= on_disconnect

	print("[GladosNode] Connecting : "+str(mqttServer)+" Port:"+str(mqttPort)+ " node: " + str(name))

	try:
		mqttClient.connect(mqttServer,port=mqttPort,keepalive=120)
	except:
		print("Cant connect, will retry automatically")
	mqttClient.loop_forever()