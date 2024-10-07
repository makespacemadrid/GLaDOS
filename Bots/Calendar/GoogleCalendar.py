# google_calendar.py
from google_auth_oauthlib.flow import InstalledAppFlow
from googleapiclient.discovery import build
from google.auth.transport.requests import Request
from google.oauth2.credentials import Credentials
import os.path
import pickle
import datetime

class GoogleCalendarClient:
    def __init__(self):
        self.creds = None
        self.scopes = ['https://www.googleapis.com/auth/calendar']

        # El archivo token.pickle almacena los tokens de acceso y actualización del usuario
        if os.path.exists('token.pickle'):
            with open('token.pickle', 'rb') as token:
                self.creds = pickle.load(token)

        # Si no hay credenciales válidas disponibles, deja que el usuario inicie sesión.
        if not self.creds or not self.creds.valid:
            if self.creds and self.creds.expired and self.creds.refresh_token:
                self.creds.refresh(Request())
            else:
                flow = InstalledAppFlow.from_client_secrets_file(
                    '/data/`credentials.json', self.scopes)
                self.creds = flow.run_local_server(port=0)
            with open('token.pickle', 'wb') as token:
                pickle.dump(self.creds, token)

        self.service = build('calendar', 'v3', credentials=self.creds)

    def get_events(self, calendar_id='primary', time_min=None, time_max=None, max_results=10):
        # Lógica para obtener eventos de Google Calendar
        events_result = self.service.events().list(calendarId=calendar_id, timeMin=time_min,
                                                  timeMax=time_max, maxResults=max_results,
                                                  singleEvents=True, orderBy='startTime').execute()
        return events_result.get('items', [])

    def create_event(self, calendar_id='primary', event_data=None):
        # Lógica para crear un nuevo evento en Google Calendar
        event = self.service.events().insert(calendarId=calendar_id, body=event_data).execute()
        return event