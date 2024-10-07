import requests
import json
import os


ollama_url=os.environ['OLLAMA_URL']
litellm_url=os.environ['LITELLM_URL']
litellm_key=os.environ['LITELLM_KEY']

def fetch_ollama_models():
#response example:
#{
#    "models": [
#        {
#            "name": "dolphin-mistral:7b",
#            "model": "dolphin-mistral:7b",
#            "modified_at": "2024-03-17T23:28:34.032Z",
#            "size": 4109870615,
#            "digest": "ecbf896611f5b38fddc717b2e5609956980dc81187031eccc238a2547378862d",
#            "details": {
#                "parent_model": "",
#                "format": "gguf",
#                "family": "llama",
#                "families": [
#                    "llama"
#                ],
#                "parameter_size": "7B",
#                "quantization_level": "Q4_0"
#            }
#        }
#    ]
#}




    url = f'{ollama_url}/api/tags'
    response = requests.get(url)
    
    if response.status_code == 200:
        data = response.json()
        for model in data.get('models', []):
            print(f"Name: {model['name']}")
            print(f"Model: {model['model']}")
            print(f"Modified At: {model['modified_at']}")
            print(f"Size: {model['size']}")
            print(f"Digest: {model['digest']}")
            print(f"Details: {model['details']}")
            print('-----------------------------')
        return data.get('models', [])
    else:
        print(f"Failed to fetch models. Status code: {response.status_code}")
        return None

def fetch_litellm_models():
#example output:
#{
#    "data": [
#        {
#            "id": "dolphin-mistral:7b",
#            "object": "model",
#            "created": 1677610602,
#            "owned_by": "openai"
#        }
#    ],
#    "object": "list"
#}
    url = f'{litellm_url}/v1/models'
    headers = {
        'Authorization': 'Bearer sk-1234makespace'
    }

    response = requests.get(url, headers=headers)
    
    if response.status_code == 200:
        data = response.json()
        for model in data['data']:
            print(f"Name: {model['id']}")
        return data['data']
    else:
        print(f"Failed to fetch OpenAI models. Status code: {response.status_code}")
        return None

def check_chatmode(model_name):
    chatmodels = ['vicuna', 'wizard-vicuna']
    no_chatmodels = ['wizardcoder','starcoder','codestral','deepseek','nomic-embed-text']
    for name in chatmodels:
        if name in model_name:
            return True
    for name in no_chatmodels:
        if name in model_name:
            return False
    return True

def add_model_to_litellm(model_name):

    input_cost_per_token = 0.0001 
    output_cost_per_token = 0.0001
    max_tokens = 4096 

    headers = { 'Authorization': f'Bearer {litellm_key}', 'Content-Type': 'application/json' }
    response=''
    
    if '16k' in model_name:
        max_tokens = 16000

    if check_chatmode(model_name):
        mode = 'chat'
        data = { 'model_name': model_name, 'litellm_params': { 'api_base': ollama_url, 'model': f"ollama_chat/{model_name}", 'input_cost_per_token': input_cost_per_token, 'output_cost_per_token': output_cost_per_token, 'max_tokens': max_tokens }, 'model_info': { 'id': model_name, 'mode': mode, 'input_cost_per_token': input_cost_per_token, 'output_cost_per_token': output_cost_per_token, 'max_tokens': max_tokens } }
        response = requests.post(f'{litellm_url}/model/new', headers=headers, json=data)
    else:
        data = { 'model_name': model_name, 'litellm_params': { 'api_base': ollama_url, 'model': f"ollama/{model_name}", 'input_cost_per_token': input_cost_per_token, 'output_cost_per_token': output_cost_per_token, 'max_tokens': max_tokens }, 'model_info': { 'id': model_name,'input_cost_per_token': input_cost_per_token, 'output_cost_per_token': output_cost_per_token, 'max_tokens': max_tokens } }
        response = requests.post(f'{litellm_url}/model/new', headers=headers, json=data)
    print(response.json())




#get ollama models
ollama_models = fetch_ollama_models()
litellm_models = fetch_litellm_models()

if ollama_models is None or litellm_models is None:
    print("Failed to fetch models")
    exit(1)
#make a list with the models that are present in ollama but not in openai
missing_models = []
for model in ollama_models:
    if model['name'] not in [model['id'] for model in litellm_models]:
        missing_models.append(model)
print("Missing models name:")

for model in missing_models:
    name=model['name']
    print(name)
    add_model_to_litellm(name)
