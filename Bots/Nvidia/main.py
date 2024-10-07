# -*- coding: utf-8 -*-
import time
import os
import platform
import subprocess
import json
from io import StringIO  # Required for redirecting stdout & stderr
import pandas as pd
from gladosMQTT import GladosMQTT

# Configuración MQTT
mqHost = os.environ.get("MQTT_HOST")
mqPort = int(os.environ.get("MQTT_PORT"))
nodeName = platform.node()

# Verificación de variables críticas
if not mqHost or not mqPort:
    print("No MQTT config!")
    exit(1)

# Temas MQTT
topic_nvidia_stats   ="nvidia/asimov/last_status"
# Instancia de GladosMQTT
glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName)
glados_mqtt.set_topics([topic_nvidia_stats])
last_reading_timestamp = 0
last_reading_wh_drawn = 0
total_kwh_drawn = 0

def get_nvidia_stats():
    try:
        command = 'nvidia-smi --query-gpu=power.draw,temperature.gpu,utilization.gpu,utilization.memory,gpu_name,driver_version,pstate,pcie.link.gen.current  --format=csv,nounits'
        output = subprocess.check_output(command, shell=True, stderr=subprocess.STDOUT, universal_newlines=True)
        
        # Reading the CSV data and converting it to a pandas DataFrame
        data = pd.read_csv(StringIO(output))
        
        # Converting the DataFrame to json format compatible with Home Assistant
        json_data = json.loads(json.dumps(data.to_dict(orient='records')))
        
        return json_data
    except Exception as e:
        glados_mqtt.debug(str(e))

def get_stats():
	global last_reading_timestamp
	global last_reading_wh_drawn
	global total_kwh_drawn

	now = time.time()
	time_delta = now - last_reading_timestamp
	kw_spent = float(time_delta/1000/3600)*float(last_reading_wh_drawn)
	total_kwh_drawn += kw_spent
	stats=get_nvidia_stats()
	last_reading_timestamp = now
	glados_mqtt.debug(stats)


# Función para manejar mensajes MQTT
def on_mqtt_message(client, userdata, msg):
	global total_kwh_drawn
	payload = msg.payload.decode('utf-8')
	if msg.topic == topic_nvidia_stats:
	#decode payload to json and extract totalKWh
		total_kwh_drawn=float(json.loads(payload)['totalKWh'])
		print("Total KWH drawn: " + str(total_kwh_drawn))


# Configurar callbacks MQTT
glados_mqtt.mqttClient.on_message = on_mqtt_message


# Inicializar y correr el loop de mensajes MQTT
if __name__ == "__main__":
	glados_mqtt.init_mqtt()
	while True:
		time.sleep(1)
		get_stats()

	
