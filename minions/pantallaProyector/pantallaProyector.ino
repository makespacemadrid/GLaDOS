/*
 Basic ESP8266 MQTT example

Ejemplo basico de nodo para esp8266 con mqtt y OTA
Basado en el ejemplo mqtt_esp8266 de la librería pubSubClient , y el basicOTA de ArduinoOTA


Puedes usar la funcion debug("Mensaje") para publicar los print por mqtt al topico en /node/'nodeID'/debug

El OTA no funciona hasta que no se reinicia el microcontrolador despues de la primera carga.

*/

#define OTA_ENABLE    // Comentar para deshabilitar ota
//#define OTA_PASSWORD  // Comentar para deshabilitar la contraseña ota, la contraseña en si se configura mas abajo en ota_password
#define MQTT_MAX_PACKET_SIZE 1024 //Fucking arduino, pasa del orden de los includes y defines

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#ifdef OTA_ENABLE
#include <ArduinoOTA.h>
#endif
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <EasyNTPClient.h>
#include "TimeLib.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "SDM.h"                                                                //https://github.com/reaper7/
#include <ctype.h>


const uint32 tiempoPersiana = 30000;
const int    pinSubir = D1;
const int    pinBajar = D2 ;


bool subiendo         = false;
bool bajando          = false;

uint32 startTime = millis();

const char* nodeid       = "pantallaProyector";
const char* ota_password =  "666666";
const char* mqtt_server  = "192.168.10.10";

const String topicDebug         = "node/" + String(nodeid) + "/debug";
const String topicStatus        = "node/" + String(nodeid) + "/status";
const String topicCommand       = "node/" + String(nodeid) + "/cmnd";
const String topicStatusNumeric = "node/" + String(nodeid) + "/percentage";

//--------------------------------------------------
//--------------------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP       _ntpUdp;
EasyNTPClient _ntpClient(_ntpUdp, "pool.ntp.org");



time_t getNtpTime()
{
  return _ntpClient.getUnixTime();
}

void subscribe_topics()
{//Aqui las subscripciones a los topicos
  client.subscribe("node/hello");
  client.subscribe(topicCommand.c_str());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Esta funcion se ejecuta cada vez que recibimos un mensaje del mqtt
  String t(topic);
  String msg;
  for(int i = 0 ; i<length ; i++)
    msg+=(char)payload[i];
  debugln("Message arrived,Topic:" + t + " msg:" + msg);

  
  if( t == topicCommand) 
  {
    if( (msg == "up") || (msg == "0") )
    {
      SUBIR();
    }
    else if((msg == "down") || (msg == "100"))
    {
      BAJAR();
    }
    else if(msg == "stop")
    {
      PARAR();
    }
  }

}

void publish(String topic, String msg, bool persist = false)
{
  if(!client.publish(topic.c_str(),msg.c_str(),persist))
    Serial.println("Error publicando, t:"+topic+" tamaño mensaje::"+String(msg.length()));
}

void debug(const String msg)
{
  Serial.print(msg);
  client.publish(topicDebug.c_str(),msg.c_str());
  client.loop();
  yield();
}

void debugln(const String msg)
{
  debug(msg+"\n");
}

String localIP()
{
  return String(WiFi.localIP()[0]) + "." +String(WiFi.localIP()[1]) + "." +String(WiFi.localIP()[2]) + "." +String(WiFi.localIP()[3]);
}

void setup_wifi() {
//  WiFi.begin("essid", "passwd");

  delay(10);
  WiFi.hostname(nodeid);
  WiFiManager wifiManager;

  if (!wifiManager.autoConnect(nodeid,"configureme")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
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
  int errCount = 0;
  while (!client.connected()) 
  {
    errCount++;
    if(errCount > 10)
    {
      Serial.print("Reseteando...");
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
      delay(5000);
    }
    handleStuff();
  }
}


void setup() {
  
  pinMode(pinSubir, OUTPUT);
  pinMode(pinBajar, OUTPUT);
  digitalWrite(pinSubir, HIGH); // HIGH ES APAGADO EN EL WEMOS
  digitalWrite(pinBajar, HIGH);
 
  Serial.begin(115200);
  Serial.println("MAX_MQTT:" + String(MQTT_MAX_PACKET_SIZE));
  setup_wifi();
  MDNS.begin(nodeid);
  _ntpUdp.begin (123);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_ota();
  Serial.print(String(F("Sync NTP Time, current:"))+String(now())+"...");
  setSyncProvider(getNtpTime);
  setSyncInterval(600);
  Serial.println(String(F("Sync:"))+String(now()));
}

void handleStuff()
{
    client.loop();
    ArduinoOTA.handle();
    yield();  
}

void PARAR()
{
  debugln("Parando");
  subiendo = false;
  bajando  = false;
  digitalWrite(pinSubir, HIGH);
  digitalWrite(pinBajar, HIGH);
  delay(250);
}

void SUBIR() {
  PARAR();
  debugln("Subir");  
  subiendo = true;
  startTime = millis();
  client.publish(topicStatus.c_str(),"goingup");
  digitalWrite(pinSubir, LOW);
}
void BAJAR() {
  /* */
  PARAR();
  debugln("Bajar");
  bajando = true;
  startTime = millis();
  client.publish(topicStatus.c_str(),"goingdown");
  digitalWrite(pinBajar, LOW);
}

void loop() {

  handleStuff();
  if(subiendo || bajando)
  {
    uint32 tiempo = millis() - startTime;
    if(subiendo)
    {
      int percent = tiempo * 100 / tiempoPersiana;
      if(percent%5 == 0) client.publish(topicStatusNumeric.c_str(),String(percent).c_str());
      if(tiempo > tiempoPersiana)
      {
        PARAR();
        client.publish(topicStatus.c_str(),"up");
      }
    } else if(bajando)
    {
      if(tiempo > tiempoPersiana)
      {
        PARAR();
        client.publish(topicStatus.c_str(),"down");
      }
    }
    handleStuff();
    return;
  }
  else
  {
    if (!client.connected()) {
      reconnect();
    }
    delay(50);    
  }
}
