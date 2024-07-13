import os
import platform
import json
import requests
from io import BytesIO
from openai import OpenAI
from gladosMQTT import GladosMQTT 

class iatools:
    def __init__(self):
        self.default_prompt = "Eres un asistente que ayuda a los usuarios dando respuestas concisas y breves"
        self.api_key  = os.environ.get('LITTLELLM_API_KEY')
        self.api_base = os.environ.get('LITTLELLM_API_BASE')
        self.default_model = os.environ.get('DEFAULT_MODEL')
        self.tts_url = os.environ.get("TTS_URL")
        self.stt_url = os.environ.get("STT_URL")
        self.litellm=OpenAI(api_key=self.api_key,base_url=self.api_base)
        self.mqtt = GladosMQTT(host=os.environ.get("MQTT_HOST"), port=int(os.environ.get("MQTT_PORT"), name=nodeName, msg_callback=self.mqtt_callback)
        self.mqttExternalCallback = None
    #DEBUG
    def debug(self,msg):
        self.mqtt.debug(msg)

    #MQTT
    def mqtt_set_topics(self,topics)
        self.mqtt.set_topics(topics)

    def mqtt_callback(self,client, userdata, msg):
        if mqttExternalCallback:
            mqttExternalCallback(client, userdata, msg)

    def publish_mqtt(self,topic,msg,qos=2,persist=False):
        self.mqtt.publish(topic, msg,qos=qos, persist=persist)

    def init_mqtt_and_loop_forever(self):
        self.mqtt.init_mqtt_and_loop_forever()
    def init_mqtt(self):
        self.mqtt.init_mqtt()

    #LLM
    def set_default_llm_model(self, model_name):
        self.default_model = model_name

    def get_available_llm_models(self):
        models = self.litellm.Model.list()
        return models

    def chatCompletion(self, prompt=None,systemPrompt=None,messages=None, model=None, maxTokens=1024, stream=False, temperature=0.5):
        if model is None:
            model = self.default_model
        if messages is None:
            messages = []
            if systemPrompt:
                messages.append({"role": "system", "content": systemPrompt})
            messages.append({"role": "user", "content": prompt})
        try:
            response = self.litellm.chat.completions.create(model=model, messages=messages, max_tokens=maxTokens, stream=stream, temperature=temperature)
            return response
        except Exception as e:
            response = {}
            response['error'] = str(e)
            return response

    #IMG2TXT
    #TODO....

    #TXT2IMG
    #TODO....

    #TTS
    def tts(self,text,language="es"):
        headers = {'Content-Type': 'application/json'}
        data = {
            "text": text,
            "language": language
        }

        try:
            tts_response = requests.post(self.tts_url, json=data, headers=headers)
            tts_response.raise_for_status()  # Esto provocará una excepción si la respuesta no es exitosa
            audio_bytes = BytesIO(tts_response.content)
            audio_bytes.name = "tts.wav"
            return audio_bytes
        except requests.RequestException as e:
            glados_mqtt.debug(f"Error al generar respuesta de audio: {e}")

    #STT
    def stt(self,audio_file_path,language="es"):
        params = {
            "encode": "true",
            "task": "transcribe",
            "language": language,
            "word_timestamps": "false",
            "output": "txt"
        }
        files = {'audio_file': (audio_file_path, open(audio_file_path, 'rb'), 'audio/wav')}

        try:
            response = requests.post(self.stt_url, params=params, files=files)
            response.raise_for_status()  # Esto provocará una excepción si la respuesta no es exitosa
            return response.text  # Retorna el contenido de la transcripción
        except requests.RequestException as e:
            glados_mqtt.debug(f"Error al enviar el archivo de audio a la API: {e}")
