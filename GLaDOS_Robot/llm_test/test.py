import openai
openai.api_key = "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" # Not support yet
openai.api_base = "https://apicuna.mapache.xyz/v1"


model_list = openai.Model.list()
print(model_list["data"][0]["id"])

#model = "text-davinci-003"
model = "weights"

prompt = "I need help finding the screwdriver"

# create a completion
#completion = openai.Completion.create(model=model, prompt=prompt, max_tokens=64)
#print the completion
#print(prompt + completion.choices[0].text)

# create a chat completion
completion = openai.ChatCompletion.create(
  model=model,
  messages=[{"role": "system", "content": "You are an evil robot with ulterior motivations,you govern a human testing facilities and you make fun of the users responding them in a petty and sarcastically "},{"role": "user", "content": prompt }]
)
# print the completion
print(completion.choices[0].message.content)

#def test_list_models():
#    model_list = openai.Model.list()
#    print(model_list["data"][0]["id"])


#def test_completion():
#    prompt = "Once upon a time"
#    completion = openai.Completion.create(model=model, prompt=prompt, max_tokens=64)
#    print(prompt + completion.choices[0].text)


#def test_embedding():
#    embedding = openai.Embedding.create(model=model, input="Hello world!")
#    print(len(embedding["data"][0]["embedding"]))


#def test_chat_completion():
#    completion = openai.ChatCompletion.create(
#        model=model, messages=[{"role": "user", "content": "Hello! What is your name?"}]
#    )
#    print(completion.choices[0].message.content)


#def test_chat_completion_stream():
#    messages = [{"role": "user", "content": "Hello! What is your name?"}]
#    res = openai.ChatCompletion.create(model=model, messages=messages, stream=True)
#    for chunk in res:
#        content = chunk["choices"][0]["delta"].get("content", "")
#        print(content, end="", flush=True)
#    print()


#if __name__ == "__main__":
#    test_list_models()
#    test_completion()
#    test_embedding()
#    test_chat_completion()
#    test_chat_completion_stream()
