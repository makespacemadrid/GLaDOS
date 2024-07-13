import json
import os

class UserContext:
    def __init__(self, master_prompt="", initial_assistant="",model_name=None):
        self.master_prompt = master_prompt
        self.initial_assistant = initial_assistant
        self.history = []
        self.lastPrompt = ""
        self.master_prompt_extra = ""
        self.model_name=model_name 
        if model_name is None :
            self.model_name=model_name = os.environ.get('DEFAULT_MODEL')

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
        combined_prompt.extend(self.history)
        return combined_prompt

    def clear_history(self):
        self.history = []

    def set_model_name(self, model_name: str):
        self.model_name = model_name

    def get_model_name(self) -> str:
        return self.model_name