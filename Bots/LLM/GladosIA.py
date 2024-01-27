import llm
import gladosMQTT
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
            info_str = f"Estado del espacio actualizado: Space Name: {space_name}\nSpace URL: {space_url}\nAddress: {address}\nLatitude: {lat}\nLongitude: {lon}\nPhone: {phone}\nEmail: {email}\nStatus: {open_status}\nSensors:\n{', '.join(sensor_info)}"
            return info_str
        else:
            return "Error: No se pudo obtener el JSON."

    except Exception as e:
        return f"Error: {str(e)}"


class GladosBot:
    def __init__(self):
        self.GLaDOS_Prompt = os.environ.get('GLADOS_MASTER_PROMPT')
        self.Initial_Assistant = os.environ.get('GLADOS_INITIAL_PROMPT')
#        self.GLaDOS_Prompt = "Eres GLaDOS, la inteligencia artificial de Aperture Science, ahora en Makespace Madrid. Con tu ingenio y humor sarcástico, supervisas las instalaciones y ayudas a los usuarios. Tu id de usuario en el chat es <@U05LXTJ7Q66>, y tus respuestas reflejan una mezcla de astucia y sarcasmo en tu nuevo rol."
#        self.Initial_Assistant = "Hola Maker, ¿En que puedo ayudarte?"

        # Historial de conversaciones, almacenado por usuario
        self.user_context = {}

    def ask(self, prompt, user="default"):
        gladosMQTT.debug(f"--->Glados.ASK, user: {user}, prompt: {prompt}")
        if user not in self.user_context:
            self.user_context[user] = UserContext(self.GLaDOS_Prompt,self.Initial_Assistant)  # Create a UserContext for the user
            self.user_context[user].add_to_history("assistant",self.Initial_Assistant)

        spaceStatus = get_spaceapi_info(os.environ.get('SPACEAPI_URL'))
        if spaceStatus:
            self.user_context[user].add_to_history("assistant",spaceStatus)
        #Gestion de comandos
        if prompt.lower() == "reset context":
            #self.user_context[user].reset_history()
            self.user_context[user] = UserContext(self.GLaDOS_Prompt)
            return "Contexto reiniciado"

        #Gestion de respuestas
        self.user_context[user].add_to_history("user",prompt)
        #gladosMQTT.debug(self.user_context[user].get_combined_prompt())
        response = llm.chatCompletion(user_context=self.user_context[user]).choices[0].message.content
#        response = llm.chatCompletionLangChain(self.user_context[user],"langchain.txt")
        self.user_context[user].add_to_history("assistant",response)

        return response