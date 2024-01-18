# SLACK Bot
import os
from flask import Flask, request, jsonify
from slack_sdk import WebClient
from slack_sdk.errors import SlackApiError


def run_bot(askGLaDOS):
    print("Connecting Slack...")
    # Inicializar cliente de Slack con tu token
    app = Flask(__name__)
    slack_token = os.environ.get("SLACK_API_TOKEN")
    slack_client = WebClient(token=slack_token)

    @app.route('/slack/events', methods=['POST'])
    def slack_events():
        data = request.json
        # Desafío de verificación de Slack
        if data.get('type') == 'url_verification':
            return jsonify({'challenge': data.get('challenge')})

        if data['type'] == 'event_callback':
            event = data['event']
            print("======SLACK EVENT=====")
            print(event)
            print("======-----------=====")
            if 'bot_id' not in event and (event['type'] == 'message'):
                try:
                    response = slack_client.chat_postMessage(
                        channel=event['channel'],
                        text=askGLaDOS(event['text'])
                    )
                except SlackApiError as e:
                    print(f"Error sending message: {e.response['error']}")
        return jsonify({'status': 'ok'}), 200

# response = slack_client.chat_postMessage(channel='#glados_testing', text="Hola, mundo!")
    app.run(host='0.0.0.0', port=3000)
