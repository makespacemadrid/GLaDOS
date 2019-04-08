/*
 Basic ESP8266 MQTT example

Ejemplo basico de nodo para esp8266 con mqtt y OTA
Basado en el ejemplo mqtt_esp8266 de la librería pubSubClient , y el basicOTA de ArduinoOTA


Puedes usar la funcion debug("Mensaje") para publicar los print por mqtt al topico en /node/'nodeID'/debug

El OTA no funciona hasta que no se reinicia el microcontrolador despues de la primera carga.

*/

#define OTA_ENABLE    // Comentar para deshabilitar ota
//#define OTA_PASSWORD  // Comentar para deshabilitar la contraseña ota, la contraseña en si se configura mas abajo en ota_password


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#ifdef OTA_ENABLE
#include <ArduinoOTA.h>
#endif

//Configurar nodeid ssid y password!!!

const char* nodeid       = "testNode";
const char* ota_password =  "666666";
const char* ssid         = "wifiname";
const char* password     = "wifipassword";
const char* mqtt_server  = "iot.eclipse.org";
//--------------------------------------------------
//--------------------------------------------------

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

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(nodeid)) 
    {
      debugln("connected");
      subscribe_topics();
      String pub = String("Hola Mundo! Soy ") + String(nodeid);
      client.publish("node/hello",pub.c_str());
    } else {
      debug("failed, rc=");
      debug(String(client.state()));
      debugln(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
 
  Serial.begin(115200);
  setup_wifi();
  MDNS.begin(nodeid);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_ota();
  
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();
  //......
}
