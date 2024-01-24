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
    gladosMQTT.debug(f"--->Chat completion: {chatHistory}")

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

    gladosMQTT.debug(f"---->LLM : {messages}")

    try:
        response = llm_mks.chat.completions.create(model=current_model, messages=messages, max_tokens=maxTokens)
        gladosMQTT.debug("========LLM OUTPUT===============")
        gladosMQTT.debug(response)
        return response
    except Exception as e:
        gladosMQTT.debug(f"Error in chatCompletion: {str(e)}")
        return None