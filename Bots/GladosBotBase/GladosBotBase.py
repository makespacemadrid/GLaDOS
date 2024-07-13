import os
import requests
from IATools import *

class userData:
    def __init__(self,channel_id,channel_name=None,user_id=None,user_name=None,channel_type="",user_type="user",audio_response=False):
        self.channel_id = channel_id
        self.channel_name = channel_name
        self.user_id = user_id
        self.user_name = user_name
        self.channel_type = channel_type
        self.user_type = user_type
        self.audio_response = False

    def channelID(self)  : return self.channel_id
    def channelName(self): return self.channel_name
    def userID(self)     : return self.user_id
    def userName(self)   : return self.user_name
    def channelType(self): return self.channel_type
    def userType(self)   : return self.user_type
    def audioResponse(self): return self.audio_response

class userContext:
    def __init__(self,userdata):
        self.userdata = userdata
        self.system_prompt = ""
        self.system_prompt_extra = ""
        self.history = []
        self.lastPrompt = ""

    def userdata(self): return self.userdata

    def add_to_history(self,role,msg):
        if agent == "user":
            self.lastPrompt = msg
        # Crea un diccionario JSON válido para el agente y el mensaje
        message_dict = {"role": role, "content": msg}
        # Agrega el mensaje al historial
        self.history.append(message_dict)      

    def add_user_msg(self,msg):
        add_to_history("user",msg)
    def add_assistant_msg(self,msg):
        add_to_history("assistant",msg)

    def set_system_prompt(self,prompt):
        self.system_prompt = prompt
    def set_system_prompt_extra(self,prompt):
        self.system_prompt_extra = prompt

    def get_full_history(self):
        return self.history

    def get_combined_history(self):
        combined_history = [{"role": "system", "content": self.system_prompt+self.system_prompt_extra}]
        combined_history.extend(self.history)
        return combined_prompt

    def clear_history(self):
        self.history = []

class GladosBot:
    def __init__(self,iatools):
        self.default_prompt="Eres un asistente que ayuda a los usuarios dando respuestas concisas y breves"
        self.iatools=iatools
        self.debug_func=iatools.debug
        self.GLaDOS_prompt=os.environ.get('GLADOS_MASTER_PROMPT')
        self.initial_assistant_prompt=os.environ.get('GLADOS_INITIAL_PROMPT')
        self.default_model=os.environ.get('DEFAULT_MODEL')
        self.spaceapi_url=os.environ.get('SPACEAPI_URL')
        self.self_channel_id=None
        self.user_context = {}

    def debug(self,msg):
        self.debug_func(msg)

    def set_self_channel_id(self,channel_id):
        self.self_channel_id=channel_id
    
    def get_user_context(self,channel_id):
        for history in self.user_context:
            if history.channelID() == channel_id: return history
        return None

    def add_new_user(self,userdata):
        n_history = userContext(userdata)
        n_history.set_system_prompt(self.GLaDOS_prompt)
        self.user_context[userdata.channelID()] = n_history


    def is_channel_id_in_history(self,channel_id):
        for history in self.user_context:
            if history.userdata.channelID() == channel_id: return True 
        return False

    def get_spaceapi_info(self,url):
        try:
            # Realizar una solicitud GET a la URL
            response = requests.get(url)

            # Verificar si la solicitud fue exitosa (código de estado 200)
            if response.status_code == 200:
                # Obtener el contenido JSON de la respuesta
                data = response.json()

                # Extraer campos relevantes
                space_name = data.get("space", "N/A")
                space_url = data.get("url", "N/A")
                location = data.get("location", {})
                address = location.get("address", "N/A")
                lat = location.get("lat", "N/A")
                lon = location.get("lon", "N/A")
                contact = data.get("contact", {})
                phone = contact.get("phone", "N/A")
                email = contact.get("email", "N/A")
                state = data.get("state", {})
                is_open = state.get("open", False)
                open_status = "Espacio Abierto" if is_open else "Espacio Cerrado"
                sensors = data.get("sensors", {})
                sensor_info = []

                for sensor_type, sensor_list in sensors.items():
                    for sensor in sensor_list:
                        sensor_name = sensor.get("name", "N/A")
                        sensor_value = sensor.get("value", "N/A")
                        sensor_unit = sensor.get("unit", "")
                        sensor_location = sensor.get("location", "N/A")
                        sensor_info.append(f"{sensor_name} ({sensor_type}, {sensor_location}): {sensor_value} {sensor_unit}")


                # Crear una cadena con los campos relevantes y sus valores, incluyendo el estado de apertura/cierre
                info_str = "-Informacion que puede ser relevante para las consultas de los usuarios:\n"
                info_str +=  f"**Informacion de contacto de MakeSpace Madrid: \nSpace URL: {space_url}\nAddress: {address}\nLatitude: {lat}\nLongitude: {lon}\nPhone: {phone}\nEmail: {email}\n "
                info_str += "**La fecha actual es: " + str(datetime.now().strftime("%d/%m/%Y"))
                info_str += f"**Informacion de estado y sensores: \n OpenStatus: {open_status}\nSensors:\n{', '.join(sensor_info)}\n"
                info_str += "**Eventos: \n todavia no hay informacion en tiempo real de los eventos. Sin embargo hay los siguientes eventos recurrentes: \n - Martes abiertos a partir de las 19\n - Jueves y Viernes 10 a 17 Impress3D - Fundacion amas\n -Taller de sintetizadores el primer jueves de cada mes."

                return info_str
            else:
                return "Error: No se pudo obtener el JSON."

        except Exception as e:
            return f"Error: {str(e)}"

    def chat(self,message,channel_id):
        user_context = self.get_user_context(channel_id)
        if not user_context:
            return "No se pudo obtener el contexto del usuario"
        user_context.add_user_msg(message)
        user_context.set_system_prompt_extra(get_spaceapi_info(self.spaceapi_url))
        msgs = user_context.get_combined_history()
        result = iatools.chatCompletion(messages=msgs)
        if error not in result:
            response = result.choices[0].message.content
            user_context.add_assistant_msg(response)
            return response
        else:
            return "Error chat"
            