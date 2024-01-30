import json

class UserContext:
    def __init__(self, master_prompt="", initial_assistant=""):
        self.master_prompt = master_prompt
        self.initial_assistant = initial_assistant
        self.history = []
        self.lastPrompt = ""
        self.master_prompt_extra = ""

    def set_system_prompt_extra(self,str):
        self.master_prompt_extra = str
        print(f"SYSTEM_EXTRA: {str}")

    def set_master_prompt(self, prompt):
        self.master_prompt = prompt

    def set_initial_assistant(self, prompt):
        self.initial_assistant = prompt

    def add_to_history(self, agent, message):
        if agent == "user":
            self.lastPrompt = message
        # Crea un diccionario JSON válido para el agente y el mensaje
        message_dict = {"role": agent, "content": message}
        # Agrega el mensaje al historial
        self.history.append(message_dict)

    def get_last_prompt(self):
        return self.lastPrompt
    
    def get_master_prompt(self):
        return self.master_prompt
    
    def get_master_prompt_extra(self):
        return self.master_prompt_extra

    def get_initial_assistant(self):
        return self.initial_assistant

    def get_history(self):
        return self.history

    def get_combined_prompt(self, max_tokens=None):
        system_prompt = {"role": "system", "content": self.master_prompt+self.master_prompt_extra}
        combined_prompt = [system_prompt]  # Inicialmente, combinamos el prompt del sistema
#        if self.master_prompt_extra:
#            message_extra = {"role": "user", "content": self.master_prompt_extra}
#            combined_prompt.append(message_extra)
        # Concatena los mensajes del historial
        combined_prompt.extend(self.history)

        # Convierte la lista de mensajes en una cadena JSON válida
        #combined_prompt_json = json.dumps(combined_prompt)

        return combined_prompt


    def clear_history(self):
        self.history = []
