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
from GoogleCalendar import GoogleCalendarClient



#Variables
mqHost	 = os.environ.get("MQTT_HOST", "mqtt.makespacemadrid.org")
mqPort 	 = os.environ.get("MQTT_PORT", 1883)
nodeName = platform.node()

slack_token = os.environ.get("SLACK_API_TOKEN")
slack_port  = os.environ.get("SLACK_PORT")



topic_calendar_events = "calendar/events"
topic_calendar_update = "calendar/update"



def subscribeTopics() :
	gladosMQTT.subscribe(topic_calendar_update)


def on_connect(client, userdata, rc,arg):
	subscribeTopics()
	updateCalendar()

def on_disconnect(client, userdata, rc):
	gladosMQTT.debug("Disconnected! rc: "+str(rc))

def on_message(client, userdata, msg):
	if (msg.topic == topic_calendar_update) :
		try:
			updateCalendar()
		except json.JSONDecodeError as e:
			gladosMQTT.debug("Error al parsear JSON:")

def updateCalendar():
	gladosMQTT.debug("Update Calendar")


calendar_client = GoogleCalendarClient('/data/credentials.json')
gladosMQTT.initMQTTandLoopForever(mqHost,mqPort,nodeName,on_connect,on_message,on_disconnect)

