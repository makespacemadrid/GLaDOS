# Telegram BOT
import os
import asyncio
from telethon import TelegramClient, events

import GladosIA


def run_bot(askGLaDOS):

    tg_api_id = os.environ.get("TELEGRAM_ID")
    tg_api_hash = os.environ.get("TELEGRAM_HASH")
    tg_bot_token = os.environ.get("TELEGRAM_TOKEN")

    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    tg_client = TelegramClient('./sess_id', tg_api_id, tg_api_hash, loop=loop)

    @tg_client.on(events.NewMessage())
    async def readMessages(event):
        user = await tg_client.get_entity(event.peer_id.user_id)
        id = user.id
        name = user.first_name
        lastName = user.last_name
        username = user.username
        print(event.message)
        # then we build the message we want to respond:
        # respondMessage = "Message sender: %s @%s, with id: %s"%( name + ' ' + lastName if lastName is not None else name, username, id )
        respondMessage = askGLaDOS(event.message.message)
        # then we send the message back replying the recibed message
        await event.reply(respondMessage)

    try:
        tg_client.start(bot_token=tg_bot_token)
        print('Telegram bot started.')
        loop.run_until_complete(tg_client.run_until_disconnected())
    finally:
        tg_client.disconnect()
