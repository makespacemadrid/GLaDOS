import openai
from openai import OpenAI
import os

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


def chatCompletion(prompt="", chatHistory="", masterPrompt="", initialAssistant="", maxTokens=256, langChainContext='none'):
    global current_model
    select_model()
    result = ''

    if (chatHistory == ''):
        if (masterPrompt == ''):
            masterPrompt = {"role": "system", "content": default_prompt}
        if (initialAssistant != ''):
            result = [masterPrompt, initialAssistant,
                      {"role": "user", "content": prompt}]
        else:
            result = [masterPrompt, {"role": "user", "content": prompt}]
    else:
        result = '['
        for msg in chatHistory:
            print(msg, flush=True)
            result += str(msg)
            result += ','
        result += str({"role": "user", "content": prompt})
        result += ']'
    print("==============Lanzando peticion al LLM=======================")
    print(result, flush=True)

    completion = llm_mks.chat.completions.create(model=current_model, messages=result, max_tokens=maxTokens)
    print("========LLM OUTPUT===============")
#    print(completion.choices[0].message.content)
    print(completion, flush=True)
    return completion
