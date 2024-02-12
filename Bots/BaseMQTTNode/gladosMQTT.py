# -*- coding: utf-8 -*-
import paho.mqtt.client as mqtt
import json

class GladosMQTT:
    def __init__(self, host="192.168.1.1", port=1883, name="test-node", msg_callback=None):
        self.mqttServer = host
        self.mqttPort = port
        self.nodeName = name
        self.baseTopic = "node/" + self.nodeName
        self.statusTopic = self.baseTopic+'/status'
        self.debugTopic = self.baseTopic + "/debug"
        self.mqttClient = mqtt.Client()
        self.topics = []
        self.nodeMsgCallback = msg_callback if msg_callback is not None else self.dummy

        self.mqttClient.on_connect = self.on_connect
        self.mqttClient.on_message = self.on_message
        self.mqttClient.on_disconnect = self.on_disconnect
        self.mqttClient.will_set(self.statusTopic,'OFFLINE', int(2), True)


    def set_topics(self, topics):
        self.topics = topics

    def dummy(self, *args, **kwargs):
        self.debug("Dummy function called")

    def publish(self, topic, msg, persist=False):
        self.mqttClient.publish(topic, msg,int(2), persist)

    def debug(self, msg):
        if not isinstance(msg, str):
            try:
                msg = json.dumps(msg)
            except (TypeError, ValueError):
                msg = str(msg)

        print(msg)
        self.publish(self.debugTopic, msg)

    def on_connect(self, client, userdata, flags, rc):
        self.debug("[GladosNode] Connected with result code " + str(rc))
        self.mqttClient.publish(self.statusTopic,'ONLINE', int(2))
        for topic in self.topics:
            self.mqttClient.subscribe(topic,int(2))

    def on_message(self, client, userdata, msg):
#        try:
#            self.debug("[GladosNode] mqtt_rcv: { " + msg.topic + " - " + msg.payload.decode() + " }")
#        except Exception as e:
#            self.debug("[GladosNode] mqtt_rcv error: " + str(e))
        self.nodeMsgCallback(client, userdata, msg)

    def on_disconnect(self, client, userdata, rc):
        self.debug("[GladosNode] Disconnected with result code " + str(rc))

    def init_mqtt(self):
        self.debug("[GladosNode] Connecting : " + str(self.mqttServer) + " Port:" + str(self.mqttPort) + " node: " + str(self.nodeName))
        try:
            self.mqttClient.connect(self.mqttServer, port=self.mqttPort, keepalive=120)
        except Exception as e:
            self.debug("Cannot connect: " + str(e))
        self.mqttClient.loop_start()

    def init_mqtt_and_loop_forever(self):
        self.debug("[GladosNode] Connecting : " + str(self.mqttServer) + " Port:" + str(self.mqttPort) + " node: " + str(self.nodeName))
        try:
            self.mqttClient.connect(self.mqttServer, port=self.mqttPort, keepalive=120)
        except Exception as e:
            self.debug("Cannot connect: " + str(e))
        self.mqttClient.loop_forever()
