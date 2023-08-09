import gladosIA

import os
from flask import Flask, request, jsonify
from slack_sdk import WebClient
from slack_sdk.errors import SlackApiError
#from slack_bolt import App, Say
#from slack_bolt.adapter.flask import SlackRequestHandler


from telethon import TelegramClient, events

slack_token  = os.environ.get("SLACK_API_TOKEN")
slack_secret = ''
tg_api_id    = os.environ.get("TELEGRAM_ID")
tg_api_hash  = os.environ.get("TELEGRAM_HASH")
tg_bot_token = os.environ.get("TELEGRAM_TOKEN")


tg_client = TelegramClient('./sess_id', tg_api_id, tg_api_hash)
# Note that 'session_name_file' will be used to save your session (persistent information such as access key and others) as 'session_name_file.session' in your disk. 
# This is by default a database file using Python's sqlite
#Gestion de eventos de telethon
@tg_client.on(events.NewMessage())
async def readMessages(event):
    # first we get the user information
    user = await tg_client.get_entity(event.peer_id.user_id)
    id = user.id
    name = user.first_name
    lastName = user.last_name
    username = user.username
    print(event.message)
    #then we build the message we want to respond:
    #respondMessage = "Message sender: %s @%s, with id: %s"%( name + ' ' + lastName if lastName is not None else name, username, id )
    respondMessage = gladosIA.askGLaDOS(event.message.message)
    #then we send the message back replying the recibed message
    await event.reply(respondMessage)



# Inicializar cliente de Slack con tu token
app = Flask(__name__)
slack_client = WebClient(token=slack_token)
#bolt_app = App(token=slack_token),
#signing_secret=slack_secret
#handler = SlackRequestHandler(bolt_app)
#@app.route("/events", methods=["POST"])
#def slack_events():
#    """ Declaring the route where slack will post a request """
#    return handler.handle(request)


#if __name__ == '__main__':
#    app.run(host='0.0.0.0', port=8080, debug=True)



try:
    tg_client.start(bot_token=tg_bot_token)
    response = slack_client.chat_postMessage(channel='#glados_testing', text="Hola, mundo!")

    print('Successfully started (Press Ctrl+C to stop)')
    tg_client.run_until_disconnected()
finally:
    tg_client.disconnect()
    # Create a log or print a message in the console indicating that the bot has stoped..


