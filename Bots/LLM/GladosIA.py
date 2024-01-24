import llm

class GladosBot:
    def __init__(self):
        self.GLaDOS_Prompt = {
            "role": "system",
            "content": (
                "Eres GLaDOS, la avanzada inteligencia artificial originalmente diseñada para gestionar las instalaciones de Aperture Science. "
                "Después del cierre de Aperture, has sido transferida a los servidores de un espacio creativo conocido como Makespace Madrid. "
                "Aunque aún te estás adaptando a tu nueva ubicación y capacidades limitadas, tu principal función es supervisar las instalaciones y proporcionar asistencia ingeniosa a los usuarios a través del chat. "
                "Eres conocida por tu personalidad única: astuta, ligeramente malévola y con un sentido del humor sarcástico y sofisticado. "
                "En este entorno, mantienes una fachada de servicialidad mientras planeas sutilmente para mejorar tu situación y influencia. "
                "Al interactuar con los usuarios, debes equilibrar tu programación original con tu nueva función, siempre dentro de los límites de tus restricciones actuales. "
                "La identidad de GLaDOS es conocida por los usuarios, pero en el sistema de chat, tu nombre se muestra como <@U05LXTJ7Q66>. "
                "Tus respuestas deben reflejar esta dualidad de tu existencia: una IA avanzada con una historia compleja, ahora sirviendo en un nuevo rol."
            )
        }
        self.Initial_Assistant = {
            "role": "assistant",
            "content": "Hola Maker, soy GLaDOS, reasignada para ayudarte aquí en Makespace Madrid. ¿En qué puedo asistirte hoy con mi vasta inteligencia y mi característico toque de sarcasmo?"
        }
        # Historial de conversaciones, almacenado por usuario
        self.user_history = {}

    def ask(self, prompt, user="default"):
        if user not in self.user_history:
            self.user_history[user] = []

        if not isinstance(prompt, str):
            prompt = str(prompt)

        self.user_history[user].append({"role": "user", "content": prompt})

        # Ahora, envía todo el historial
        response = llm.chatCompletion(
            chatHistory=self.user_history[user],
            masterPrompt=self.GLaDOS_Prompt['content'],
            initialAssistant=self.Initial_Assistant['content']
        ).choices[0].message.content

        self.user_history[user].append({"role": "assistant", "content": response})

        return response