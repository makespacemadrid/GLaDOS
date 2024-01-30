import llm
from UserContext import UserContext  # Import UserContext
import os
import requests

def get_spaceapi_info(url):
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
            info_str = f"Reporte actualizado del sistema domotico con el estado del espacio: Space Name: {space_name}\nSpace URL: {space_url}\nAddress: {address}\nLatitude: {lat}\nLongitude: {lon}\nPhone: {phone}\nEmail: {email}\nStatus: {open_status}\nSensors:\n{', '.join(sensor_info)}"
            return info_str
        else:
            return "Error: No se pudo obtener el JSON."

    except Exception as e:
        return f"Error: {str(e)}"


class GladosBot:
    def __init__(self,debug=None):
        self.GLaDOS_Prompt = os.environ.get('GLADOS_MASTER_PROMPT')
        self.Initial_Assistant = os.environ.get('GLADOS_INITIAL_PROMPT')
        self.debug = debug if debug else self.default_debug  # Utiliza la función de depuración por defecto si no se proporciona una

        # Historial de conversaciones, almacenado por usuario
        self.user_context = {}

    def default_debug(self, message):
        # Función de depuración por defecto que imprime en la terminal
        print(f"DEBUG: {message}")

    def ask(self, prompt, user="default"):
        if user not in self.user_context:
            self.user_context[user] = UserContext(self.GLaDOS_Prompt,self.Initial_Assistant)  # Create a UserContext for the user
            self.user_context[user].add_to_history("assistant",self.Initial_Assistant)
        extraPrompt = "informacion que puede ser relevante en las consultas posteriores: \n"
        spaceStatus = get_spaceapi_info(os.environ.get('SPACEAPI_URL'))
        if spaceStatus:
            extraPrompt += spaceStatus
        self.user_context[user].set_system_prompt_extra(extraPrompt)



        #Gestion de comandos
        if prompt.lower() == "reset context":
            #self.user_context[user].reset_history()
            self.user_context[user] = UserContext(self.GLaDOS_Prompt)
            return "Contexto reiniciado"
        elif prompt.lower() == "dump context":
            return self.user_context[user].get_combined_prompt()


        #Gestion de respuestas
        self.user_context[user].add_to_history("user",prompt)
        #gladosMQTT.debug(self.user_context[user].get_combined_prompt())
        response = llm.chatCompletion(user_context=self.user_context[user]).choices[0].message.content
#        response = llm.chatCompletionLangChain(self.user_context[user],"langchain.txt")
        self.user_context[user].add_to_history("assistant",response)

        return response