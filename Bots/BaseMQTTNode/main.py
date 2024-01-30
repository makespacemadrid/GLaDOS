# -*- coding: utf-8 -*-

import os
import platform
import time
from gladosMQTT import GladosMQTT 

# Variables
mqHost = os.environ.get("MQTT_HOST")
mqPort = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()

# Definición de los callbacks
def on_mqtt_message(client, userdata, msg):
    if msg.topic == "my_topic":
        glados_mqtt.debug("msg:" + str(msg.payload))

# Instanciación de la clase GladosMQTT
glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName, msg_callback=on_mqtt_message)

# Suscripción a tópicos
glados_mqtt.set_topics(["node/topic", "my_topic"])  # Asegúrate de incluir los tópicos correctos aquí

# Inicialización de MQTT
glados_mqtt.init_mqtt_and_loop_forever()

#glados_mqtt.init_mqtt
#try:
#    while True:
        # Loop principal del programa
#        time.sleep(10)
#except KeyboardInterrupt:
#    print('Interrupted!')
