from openai import OpenAI
import os
import json

from langchain.chat_models import ChatOpenAI
from langchain.document_loaders import TextLoader
from langchain.embeddings import OpenAIEmbeddings
from langchain.indexes import VectorstoreIndexCreator
from EmbeddingManager import EmbeddingManager


openai_api_key = os.environ.get('OPENAI_API_TOKEN')
openai_api_url = os.environ.get('OPENAI_API_ENDPOINT')
custom_api_key = os.environ.get('MKSLLM_API_TOKEN')
custom_api_url = os.environ.get('MKSLLM_API_ENDPOINT')


llm_openai = OpenAI(api_key=openai_api_key,base_url=openai_api_url)
llm_mks    = OpenAI(api_key=custom_api_key,base_url=custom_api_url)

#gladosMQTT.debug("---->Creando Embeddings...")
#embedding = OpenAIEmbeddings(model="text-embedding-ada-002")
#loader = TextLoader("langchain.txt")
#index = VectorstoreIndexCreator(embedding=embedding).from_loaders([loader])
#llm_langchain = ChatOpenAI(model="gpt-3.5-turbo")
#gladosMQTT.debug("DONE!")

current_model = "none"
default_prompt = "Eres un asistente que ayuda a los usuarios dando respuestas concisas y breves"


def list_files_in_directory(directory_path):
    """ Lista todos los archivos en un directorio dado. """
    file_paths = []
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            file_path = os.path.join(root, file)
            file_paths.append(file_path)
    return file_paths



def select_model():
    model_list = llm_mks.models.list()
    global current_model
    current_model = model_list.data[0].id
    print(model_list.data)
    print("Selected model:")
    print(current_model)
    return model_list

my_embeddings = EmbeddingManager()
my_embeddings.ingest(list_files_in_directory('/data'))  # Rutas a los archivos de texto

#def chatCompletionLangChain(user_context,FileName):
#    chatHistory = user_context.get_combined_prompt()
#
#    try:
#        response = index.query(json.dumps(chatHistory), llm=llm_langchain)
#        search_results = my_embeddings.search(user_context.get_last_prompt())
#        gladosMQTT.debug(json.dumps(search_results, indent=4))  # Imprimir resultados formateados
#        response=json.dumps(search_results, indent=4)
#        gladosMQTT.debug(f"----->LLM OUTPUT: {response}")
#        return response
#    except Exception as e:
#        gladosMQTT.debug(f"Error in chatCompletion: {str(e)}")
#        return None

def chatCompletion(prompt="", user_context=None, masterPrompt="", initialAssistant="", maxTokens=512):
    global current_model
    select_model()

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
#        hist=user_context.get_combined_prompt()
#        gladosMQTT.debug(f"hist: {hist}")
#        messages.append(hist)
        for msg in user_context.get_combined_prompt():
            messages.append(msg)

    try:
        response = llm_mks.chat.completions.create(model=current_model, messages=messages, max_tokens=maxTokens)
        return response
    except Exception as e:
        return None
    

