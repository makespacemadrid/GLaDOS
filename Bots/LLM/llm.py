from openai import OpenAI
import os
import json


class LLM:
    def __init__(self, default_model="text-davinci-003"):
        self.default_prompt = "Eres un asistente que ayuda a los usuarios dando respuestas concisas y breves"
        self.api_key  = os.environ.get('LITTLELLM_API_KEY')
        self.api_base = os.environ.get('LITTLELLM_API_BASE')
        self.default_model = os.environ.get('DEFAULT_MODEL')
        self.litellm=OpenAI(api_key=self.api_key,base_url=self.api_base)

    def set_default_model(self, model_name):
        self.default_model = model_name

    def get_available_models(self):
        models = self.litellm.Model.list()
        return models

    def chatCompletion(self, prompt="", user_context=None, masterPrompt="", initialAssistant="", model=None, maxTokens=1024, stream=False, temperature=0.5):
        if model is None:
            model = self.default_model
        if user_context :
            model = user_context.get_model_name()
        messages = []
        if user_context is None:
            # Usar prompts individuales
            if masterPrompt:
                messages.append({"role": "system", "content": masterPrompt})
            if initialAssistant:
                messages.append({"role": "assistant", "content": initialAssistant})
            messages.append({"role": "user", "content": prompt})
        else:
            # Usar historial
            for msg in user_context.get_combined_prompt():
                messages.append(msg)

        try:
            response = self.litellm.chat.completions.create(model=model, messages=messages, max_tokens=maxTokens, stream=stream, temperature=temperature)
            return response
        except Exception as e:
            response = {}
            response['error'] = str(e)
            return response
