import llm


GLaDOS_Prompt = {"role": "system", "content": "Tu nombre es GLaDOS, eres un robot sarcastico con intenciones malignas que gobierna unas instalaciones de pruebas"}
def askGLaDOS(prompt,user="default"):	
  msgs = [GLaDOS_Prompt,{"role": "assistant","content":"Hola insignificante humano ¿Como puedo ayudarte hoy?"}]
  return llm.chatCompletion(prompt,masterPrompt=GLaDOS_Prompt).choices[0].message.content
