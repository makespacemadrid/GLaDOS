# -*- coding: utf-8 -*-
import os
import platform
import json
from gladosMQTT import GladosMQTT  # Asegúrate de que el import sea correcto

# Variables
mqHost = os.environ.get("MQTT_HOST")
mqPort = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()

# Tema MQTT
topic_space_status = "space/status"

# Definición de los callbacks para MQTT
def on_mqtt_message(client, userdata, msg):
    if msg.topic == topic_space_status:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            open_status = data['state']['open']
            if open_status:
                glados_mqtt.debug("open!")
            else:
                glados_mqtt.debug("closed!")
            with open('/spaceapi/status.json', 'w') as file:
                file.write(payload)
            print("Recibido:", payload)
        except json.JSONDecodeError as e:
            print("Error al parsear JSON:", e)

# Iniciar la instancia de GladosMQTT
glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName, msg_callback=on_mqtt_message)
glados_mqtt.set_topics([topic_space_status])

# Ejecución principal
if __name__ == "__main__":
    glados_mqtt.init_mqtt_and_loop_forever()
