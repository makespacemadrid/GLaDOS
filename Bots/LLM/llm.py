import openai
from openai import OpenAI
import os
import gladosMQTT
import json


openai_api_key = os.environ.get('OPENAI_API_TOKEN')
openai_api_url = os.environ.get('OPENAI_API_ENDPOINT')
custom_api_key = os.environ.get('MKSLLM_API_TOKEN')
custom_api_url = os.environ.get('MKSLLM_API_ENDPOINT')


llm_openai = OpenAI(api_key=openai_api_key,base_url=openai_api_url)
llm_mks    = OpenAI(api_key=custom_api_key,base_url=custom_api_url)



current_model = "none"
default_prompt = "Eres un asistente que ayuda a los usuarios dando respuestas concisas y breves"



def select_model():
    model_list = llm_mks.models.list()
    global current_model
    current_model = model_list.data[0].id
    print(model_list.data)
    print("Selected model:")
    print(current_model)
    return model_list


def chatCompletion(prompt="", chatHistory=None, masterPrompt="", initialAssistant="", maxTokens=256):
    global current_model
    select_model()

    messages = []

    if chatHistory is None:
        # Usar prompts individuales
        if masterPrompt:
            messages.append({"role": "system", "content": masterPrompt})
        if initialAssistant:
            messages.append({"role": "assistant", "content": initialAssistant})
        messages.append({"role": "user", "content": prompt})
    else:
        # Usar historial
        messages.extend(chatHistory)
        messages.append({"role": "user", "content": prompt})

    gladosMQTT.debug("==============Lanzando peticion al LLM=======================")
    gladosMQTT.debug(messages)

    try:
        # Usar la nueva API
        response = openai.Completion.create(
            engine=current_model,
            prompt=messages,
            max_tokens=maxTokens,
            stop=None
        )
        gladosMQTT.debug("========LLM OUTPUT===============")
        gladosMQTT.debug(response)
        return response
    except Exception as e:
        gladosMQTT.debug(f"Error in chatCompletion: {str(e)}")
        return None