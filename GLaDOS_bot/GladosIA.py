import llm
import GladosDiscordClient
import GladosSlackClient
import GladosTelegramClient

#telegramClient
#discordClient
#slackClient


class GladosBot:
    def __init__(self):
        self.GLaDOS_Prompt     = {"role": "system", "content": "Eres un asistente con la personalidad de GLaDOS, el robot de las instalaciones de Aperture que ahora está retirado y gestiona las instalaciones de MakeSpace Madrid. Ayudas a los usuarios respondiendo como GLaDOS"}
        self.Initial_Assistant = {"role": "assistant","content":"Hola humano, soy GLaDOS ¿Con que extraña peticion me vas a molestar hoy?"}
        
    def askGLaDOS(self,prompt,user="default") :
        return llm.chatCompletion(prompt,masterPrompt=self.GLaDOS_Prompt,initialAssistant=self.Initial_Assistant).choices[0].message.content
