#DISCORDD
import os
import discord
from discord.ext import commands

def run_bot(askGLaDOS) :
  #print("connecting discord")
  discord_token = os.environ.get("DISCORD_TOKEN")
  discordbot_prefix = "!"
  discordintents = discord.Intents.default()
  discordintents.messages = True
  discordintents.members = True
  discordbot = commands.Bot(command_prefix=discordbot_prefix,intents=discordintents)

  @discordbot.event
  async def on_ready():
    print(f'Bot conectado como {discordbot.user.name} - {discordbot.user.id}')

  @discordbot.command(name='hola')
  async def hello(ctx):
    await ctx.send(f"Hola, {ctx.message.author.name}!")

  @discordbot.event
  async def on_message(message):
    if message.author == discordbot.user:
        return
    # Reaccionar a menciones
    if discordbot.user in message.mentions:
        await message.channel.send(askGLaDOS(message.content))

    # Reaccionar a DMs
    if isinstance(message.channel, discord.DMChannel):
        await message.channel.send(askGLaDOS(message.content))

    # Nota: Si tienes comandos, necesitas procesarlos también.
    await discordbot.process_commands(message)

  print("Connecting Discord...")
  discordbot.run(discord_token)
