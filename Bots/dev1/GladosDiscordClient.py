# DISCORDD
import os
import discord
from discord.ext import commands


class GladosDiscordClient:
    def __init__(self):
        self.discord_token = os.environ.get("DISCORD_TOKEN")
        self.discordbot_prefix = "!"

        self.discordintents = discord.Intents.default()
        self.discordintents.messages = True
        self.discordintents.members = True
        self.discordintents.message_content = True

        self.bot = commands.Bot(
            command_prefix=self.discordbot_prefix, intents=self.discordintents)
        self._setup_bot()
        self.msgBuffer = []

    def _setup_bot(self):
        @self.bot.event
        async def on_ready():
            print(
                f'Bot conectado como {self.bot.user.name} - {self.bot.user.id}')

        @self.bot.command(name='hola')
        async def hello(ctx):
            await ctx.send(f"Hola, {ctx.message.author.name}!")

        @self.bot.event
        async def on_message(message):
            if message.author == self.bot.user:
                return

            # Reaccionar a menciones
            if self.bot.user in message.mentions:
                self.msgBuffer.append(message.content)

            # Reaccionar a DMs
            if isinstance(message.channel, discord.DMChannel):
                self.msgBuffer.append(message.content)

            # Nota: Si tienes comandos, necesitas procesarlos también.
            await self.bot.process_commands(message)

    def run(self):
        print("Connecting Discord...")
        self.bot.run(self.discord_token)

    async def sendMsg(self, msg, channel="general"):
        print("Looking discord channel: "+channel)
        print(self.bot.guilds[0].text_channels)
        ch = discord.utils.get(self.bot.guilds[0].text_channels, name=channel)
        await ch.send(msg)

    def getMsgs(self):
        result = self.msgBuffer
        self.msgBuffer = []
        return result
