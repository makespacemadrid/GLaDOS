/*
 Controlador de los cerrojos de MakeSpace
*/
#define OTA_ENABLE    // Comentar para deshabilitar ota
//#define OTA_PASSWORD  // Comentar para deshabilitar la contraseña ota, la contraseña en si se configura mas abajo en ota_password

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define PIN_CSup D5
#define PIN_CInf D2

//********************** OTA *****************
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#ifdef OTA_ENABLE
#include <ArduinoOTA.h>
#endif
//********************************************
// variables
long lastMsg = 0;
char msg[50];
int value = 0;

//Datos cerrojos

byte cerrojoSup = HIGH, cerrojoSup_Prev = HIGH; //HIGH = cerrojo abierto
byte cerrojoInf = HIGH, cerrojoInf_Prev = HIGH;
boolean alarma = false;  //alarma que indica mismo estado en los dos cerrojos
char estado[10];

//*************************************************
//Configurar nodeid ssid y password!!!

const char* nodeid       = "sensorCerrojos"; //asignar un nombre al modulo a actualizar
const char* ota_password = "666666";
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);

void subscribe_topics()

{//Aqui las subscripciones a los topicos
  client.subscribe("node/hello");
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Esta funcion se ejecuta cada vez que recibimos un mensaje del mqtt
  String t(topic);
  String msg;
  for(int i = 0 ; i<length ; i++)
    msg+=(char)payload[i];
  debugln("Message arrived,Topic:" + t + " msg:" + msg);

//  if( topic == "node/topico")
//  {
//    debugln(msg);
//  }
}

void debug(const String msg)
{
  Serial.print(msg);
  String topic = "node/" + String(nodeid) + "/debug";
  client.publish(topic.c_str(),msg.c_str());
  client.loop();
  yield();
}

void debugln(const String msg)
{
  debug(msg+"\n");
}


void setup_wifi() {

  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname(nodeid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String localIP()
{
  return String(WiFi.localIP()[0]) + "." +String(WiFi.localIP()[1]) + "." +String(WiFi.localIP()[2]) + "." +String(WiFi.localIP()[3]);
}

//***************************** OTA **************
void setup_ota()
{
  ArduinoOTA.setHostname(nodeid);
  #ifdef OTA_PASSWORD
  ArduinoOTA.setPassword(ota_password);
  #endif
  ArduinoOTA.onStart([]() {
    debugln("going OTA....");
    pinMode(LED_BUILTIN,OUTPUT);
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    debugln("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    debugln("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
    debug("Progress: ");
    debugln(String(progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    debug("Error:");
    if (error == OTA_AUTH_ERROR) {
      debugln("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      debugln("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      debugln("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      debugln("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      debugln("End Failed");
    }
  });
  ArduinoOTA.begin();
}

//**************************************************

void reconnect()
{
  // Loop until we're reconnected


  int errores =0;
  while (!client.connected())
  {
    errores++;
    if(errores > 15)
    {
      delay(3000);
      ESP.reset();
      delay(5000);
    }
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(nodeid))
    {
      debugln("connected");
      subscribe_topics();
      String pub = String("Hola Mundo! Soy ") + String(nodeid) + ", mi IP es " + localIP();
      client.publish("node/hello",pub.c_str());
    } else {
      debug("failed, rc=");
      debug(String(client.state()));
      debugln(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(3000);
      ArduinoOTA.handle();      //inicializar OTA
    }
  }
}

 //**************************************************************
 // publica a través del brooker MQTT el estado (abierto/cerrado)
 // de los cerrojos
 //**************************************************************

void publicar_cerrojo (int posicion, byte cerr_abierto) {

  alarma = true;       // un cerrojo ha cambiado de estado

  if (cerr_abierto == LOW) {snprintf (estado, 10, "cerrado");
  }else{
    snprintf (estado, 10, "abierto");
    }
  switch (posicion){
    case 1:
    snprintf(msg,50,"%s = %s","Cerrojo superior",estado);
    break;
    default:
    snprintf(msg,50,"%s = %s","Cerrojo inferior",estado);
    break;
    }
  client.publish("MakeSpace/cerrojos", msg, true); //publicacion del estado

  }

 //*******************************************************
 // publica que los dos cerrojos estan abiertos o cerrados
 //*******************************************************

void publicar_alarma(byte tipo) {
  if (tipo == HIGH) {
    snprintf (msg, 40, "los dos abiertos");
  }else{
    snprintf (msg, 40, "los dos cerrados");
  }
  client.publish("MakeSpace/cerrojos", msg, true);
  alarma = false;
}

void setup() {

  pinMode(PIN_CSup, INPUT);     // Inicializa pin cerrojo superior como input
  pinMode(PIN_CInf, INPUT);     // Inicializa pin cerrojo inferior como input

  Serial.begin(115200);

  //*************************************************************
  setup_ota();                // preparar OTA
  //*************************************************************

  setup_wifi();
  MDNS.begin(nodeid);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  //*****************************************************************
  ArduinoOTA.handle();      //inicializar OTA
  //*****************************************************************

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  cerrojoSup = digitalRead(PIN_CSup); // lee estado del cerrojo superior
  if (cerrojoSup != cerrojoSup_Prev) {
      publicar_cerrojo(1, cerrojoSup);
      cerrojoSup_Prev = cerrojoSup;
  }

  cerrojoInf = digitalRead(PIN_CInf);  //lee estado del cerrojo inferior
  if (cerrojoInf != cerrojoInf_Prev) {
      publicar_cerrojo(2, cerrojoInf);
      cerrojoInf_Prev = cerrojoInf;
  }

  if (alarma && (cerrojoSup_Prev == HIGH) && (cerrojoInf_Prev == HIGH)) publicar_alarma(HIGH);
  if (alarma && (cerrojoSup_Prev == LOW) && (cerrojoInf_Prev == LOW)) publicar_alarma(LOW);

}
