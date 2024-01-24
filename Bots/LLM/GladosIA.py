import llm
import gladosMQTT

class GladosBot:
    def __init__(self):
        self.GLaDOS_Prompt = {
            "role": "system",
            "content": (
                "Eres GLaDOS, la inteligencia artificial de Aperture Science, ahora en Makespace Madrid. "
                "Con tu ingenio y humor sarcástico, supervisas las instalaciones y ayudas a los usuarios. "
                "Mantienes tu personalidad única mientras te adaptas a tu nuevo entorno y limitaciones. "
                "Tu id de usuario en el char es <@U05LXTJ7Q66>, y tus respuestas reflejan una mezcla de astucia y sarcasmo en tu nuevo rol."
            )
        }
        self.Initial_Assistant = {
            "role": "assistant",
            "content": "Hola Maker, soy GLaDOS en Makespace Madrid. ¿Cómo puedo ayudarte con un toque de inteligencia y sarcasmo hoy?"
        }
        # Historial de conversaciones, almacenado por usuario
        self.user_history = {}

    def ask(self, prompt, user="default"):
        gladosMQTT.debug(f"--->Glados.ASK, user: {user}, prompt: {prompt}")

        if user not in self.user_history:
            self.user_history[user] = []
            self.user_history[user].append({"role": "system", "content": self.GLaDOS_Prompt['content']})
            self.user_history[user].append({"role": "assistant", "content": self.Initial_Assistant['content']})

        # Agregar el mensaje del usuario al historial de este usuario
        self.user_history[user].append({"role": "user", "content": prompt})

        response = llm.chatCompletion(chatHistory=self.user_history[user]).choices[0].message.content
        self.user_history[user].append({"role": "assistant", "content": response})

        return response
