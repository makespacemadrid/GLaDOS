# -*- coding: utf-8 -*-
import os
import platform
import json
#from gladosMQTT import GladosMQTT
from flask import Flask, request, jsonify, send_file
import torch
from TTS.api import TTS
from TTS.utils.synthesizer import Synthesizer
from io import BytesIO
import soundfile as sf
import tempfile

# Configuración
#mqHost = os.environ.get("MQTT_HOST")
#mqPort = int(os.environ.get("MQTT_PORT"))
#nodeName = platform.node()

tts_port  = os.environ.get("TTS_PORT")
tts_model = os.environ.get("TTS_MODEL")

# Temas MQTT
#topic_spaceapi = "space/status" 
#topic_report_space_open = "space/report_open"


# Instancia de GladosMQTT
#glados_mqtt = GladosMQTT(host=mqHost, port=mqPort, name=nodeName)
#glados_mqtt.set_topics([topic_spaceapi, topic_last_open_status, topic_slack_send_msg_id, topic_slack_send_msg_name,topic_slack_edit_msg])


# Get device
device = "cuda" if torch.cuda.is_available() else "cpu"
# List available 🐸TTS models
print(TTS().list_models())
# Init TTS
tts = TTS(tts_model).to(device)

# Carga el modelo y el vocoder
model_path = "/data/glados.pth"  # Actualiza esto con la ruta de tu modelo
config_path = "/data/glados_tts.json"  # Actualiza esto con la ruta de tu configuración
#synthesizer = Synthesizer(model_path, config_path)

app = Flask(__name__)

# Función para manejar mensajes MQTT
def on_mqtt_message(client, userdata, msg):
    global last_open_status
    global report_open

    # Manejar mensaje de SpaceAPI
    if msg.topic == topic_spaceapi:
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            open_status = data['state']['open']
            openSpace(open_status)
        except json.JSONDecodeError as e:
            glados_mqtt.debug("Error al parsear JSON: " + str(e))



@app.route('/synthesize', methods=['POST'])
def synthesize():
    data = request.json
    text = data.get('text', '')
    language = data.get('language', '')

    if not text:
        return "No text provided", 400

    if not language:
        language = 'es'
    wav = tts.tts(text=text, speaker_wav='/data/glados.wav', language=language)
#    wav = synthesizer.tts(text)
    # Guarda el archivo de audio temporalmente
    with tempfile.NamedTemporaryFile(delete=False, suffix='.wav') as tmp:
        sf.write(tmp, wav, 22050)  # Asumiendo que la frecuencia de muestreo es 22050 Hz
        tmp_path = tmp.name  # Guarda la ruta del archivo temporal

    # Envía el archivo como respuesta
    response = send_file(tmp_path, as_attachment=True, download_name="glados_response.wav", mimetype='audio/wav')

    # Elimina el archivo temporal después de enviarlo
    os.remove(tmp_path)

    return response


if __name__ == "__main__":
#    glados_mqtt.init_mqtt()
    app.run(host='0.0.0.0', port=tts_port,debug=False)