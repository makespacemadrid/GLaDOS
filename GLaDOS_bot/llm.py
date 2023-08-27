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


def chatCompletion(prompt="",chatHistory="",masterPrompt="",initialAssistant="",maxTokens=256,langChainContext='none') : 
  global current_model
  select_model()
  result =''

  if(chatHistory == ''):  
    if(masterPrompt == ''): 
      masterPrompt = {"role": "system", "content": default_prompt}
    if(initialAssistant != ''):
      result = [masterPrompt , initialAssistant , {"role": "user", "content": prompt }]
    else :
      result = [masterPrompt,{"role": "user", "content": prompt }]
  else :
    result='['
    for msg in chatHistory :
      print(msg, flush=True)
      result += str(msg)
      result += ','
    result += str({"role": "user", "content": prompt })
    result += ']'
  print("==============Lanzando peticion al LLM=======================")
  print(result, flush=True)
  
  completion = openai.ChatCompletion.create(
    model=current_model,messages=result,max_tokens=maxTokens)
  print("========LLM OUTPUT===============")  
#  print(completion.choices[0].message.content)
  print(completion, flush=True)
  return completion
