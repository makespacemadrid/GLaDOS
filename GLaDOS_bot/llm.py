import openai

openai_api_key = "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"
openai_api_base = "https://apicuna.mapache.xyz/v1"
custom_api_key = "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"
custom_api_base = "https://apicuna.mapache.xyz/v1"


openai.api_key = custom_api_key
openai.api_base = custom_api_base

current_model = "none"
default_prompt = "Eres un asistente que ayuda a los usuarios dando respuestas concisas y breves"
  
def set_llm_api(url,key) :
  openai.api_key = key
  openai.api_base = url
  select_model()

def select_model() :
  model_list = openai.Model.list()
  global current_model
  current_model = model_list["data"][0]["id"]
#  print(model_list["data"])
  print("Selected model:")
  print(current_model)
  return model_list


def chatCompletion(prompt="",chatHistory="",masterPrompt="",maxTokens=128) : 
  global current_model
  select_model()
  result = []

  if(chatHistory == ''):  
    if(masterPrompt == ''): 
      masterPrompt = {"role": "system", "content": default_prompt}
    result = [masterPrompt,{"role": "user", "content": prompt }]
  else :
    for msg in chatHistory :
      result += msg
    result += {"role": "user", "content": prompt }
  print("==============Lanzando peticion al LLM=======================")
  print(result)
  
  completion = openai.ChatCompletion.create(
    model=current_model,messages=result,max_tokens=maxTokens)
  print("========LLM OUTPUT===============")  
#  print(completion.choices[0].message.content)
  print(completion)
  return completion
