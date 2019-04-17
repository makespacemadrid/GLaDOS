#include <Arduino.h>


/*
 Basic ESP8266 MQTT example

Ejemplo basico de nodo para esp8266 con mqtt y OTA
Basado en el ejemplo mqtt_esp8266 de la librería pubSubClient , y el basicOTA de ArduinoOTA


Puedes usar la funcion debug("Mensaje") para publicar los print por mqtt al topico en /node/'nodeID'/debug

El OTA no funciona hasta que no se reinicia el microcontrolador despues de la primera carga.

*/

//#define OTA_ENABLE    // Comentar para deshabilitar ota
//#define OTA_PASSWORD  // Comentar para deshabilitar la contraseña ota, la contraseña en si se configura mas abajo en ota_password


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <EasyNTPClient.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "SDM.h"                                                                //https://github.com/reaper7/

const char* nodeid       = "powerMeter";
const char* ota_password =  "666666";
const char* mqtt_server  = "10.0.1.254";
//--------------------------------------------------
//--------------------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);
SoftwareSerial swSerSDM(D5, D6);                                                //config SoftwareSerial (rx->pin13 / tx->pin15) (if used)
SDM sdm(swSerSDM, 9600, NOT_A_PIN);                                             //SOFTWARE SERIAL
//SDM sdm(Serial, 9600, NOT_A_PIN, SERIAL_8N1, false);                            //HARDWARE SERIAL
WiFiUDP       _ntpUdp;
EasyNTPClient _ntpClient(_ntpUdp, "pool.ntp.org");



time_t getNtpTime()
{
  return _ntpClient.getUnixTime();
}


void debug(const String msg)
{
  //Serial.print(msg);
  String topic = "node/" + String(nodeid) + "/debug";
  client.publish(topic.c_str(),msg.c_str());
  client.loop();
  yield();
}

void debugln(const String msg)
{
  debug(msg+"\n");
}

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

void publish(String topic, String msg, bool persist = false)
{
  client.publish(topic.c_str(),msg.c_str(),persist);
}

void setup_wifi() {

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
}

String localIP()
{
  return String(WiFi.localIP()[0]) + "." +String(WiFi.localIP()[1]) + "." +String(WiFi.localIP()[2]) + "." +String(WiFi.localIP()[3]);
}

void setup_ota()
{

}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
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
  }
}


void setup() {

  Serial.begin(115200);
  setup_wifi();
  _ntpUdp.begin (123);
  MDNS.begin(nodeid);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_ota();

  Serial.print(String(F("Sync NTP Time, current:"))+String(now())+"...");
  setSyncProvider(getNtpTime);
  setSyncInterval(600);
  Serial.println(String(F("Sync:"))+String(now()));
}

void readSDM230()
{
  String topic = "node/"+String(nodeid)+"/";

  publish(topic+"volts"        ,String(sdm.readVal(SDM230_VOLTAGE)));
  publish(topic+"amps"         ,String(sdm.readVal(SDM230_CURRENT)));
  publish(topic+"power"        ,String(sdm.readVal(SDM230_POWER)));
  publish(topic+"activePower"  ,String(sdm.readVal(SDM230_ACTIVE_APPARENT_POWER)));
  publish(topic+"reactivepower",String(sdm.readVal(SDM230_REACTIVE_APPARENT_POWER)));
  publish(topic+"powerFactor"  ,String(sdm.readVal(SDM230_POWER_FACTOR)));

  publish(topic+"totalEnergy"           ,String(sdm.readVal(SDM230_TOTAL_ACTIVE_ENERGY)));
  publish(topic+"totalReactiveEnergy"   ,String(sdm.readVal(SDM230_TOTAL_REACTIVE_ENERGY)));
  publish(topic+"parcialEnergy"         ,String(sdm.readVal(SDM230_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY)));
  publish(topic+"parcialReactiveEnergy" ,String(sdm.readVal(SDM230_CURRENT_RESETTABLE_TOTAL_REACTIVE_ENERGY)));
  client.loop();


  DynamicJsonDocument json(2048);

  json[F("timestamp")] = now();
  json[F("volts")]     = sdm.readVal(SDM230_VOLTAGE);
  json[F("amps")]      = sdm.readVal(SDM230_CURRENT);
  json[F("watts")]     = sdm.readVal(SDM230_POWER);
  json[F("power")]     = sdm.readVal(SDM230_POWER);

  json[F("activePower")]   = sdm.readVal(SDM230_ACTIVE_APPARENT_POWER);
  json[F("reactivePower")] = sdm.readVal(SDM230_REACTIVE_APPARENT_POWER);
  json[F("powerFactor")]   = sdm.readVal(SDM230_POWER_FACTOR);
  json[F("phaseAngle")]    = sdm.readVal(SDM230_PHASE_ANGLE);
  json[F("frecuency")]     = sdm.readVal(SDM230_FREQUENCY);

  json[F("importActive")]   = sdm.readVal(SDM230_IMPORT_ACTIVE_ENERGY);
  json[F("exportActive")]   = sdm.readVal(SDM230_EXPORT_ACTIVE_ENERGY);
  json[F("importReactive")] = sdm.readVal(SDM230_IMPORT_REACTIVE_ENERGY);
  json[F("exportReactive")] = sdm.readVal(SDM230_EXPORT_REACTIVE_ENERGY);

  json[F("totalDemand")]           = sdm.readVal(SDM230_TOTAL_SYSTEM_POWER_DEMAND);
  json[F("maxDemand")]             = sdm.readVal(SDM230_MAXIMUM_SYSTEM_POWER_DEMAND);
  json[F("positiveDemand")]        = sdm.readVal(SDM230_CURRENT_POSITIVE_POWER_DEMAND);
  json[F("maxPositiveDemand")]     = sdm.readVal(SDM230_MAXIMUM_POSITIVE_POWER_DEMAND);
  json[F("reverseDemand")]         = sdm.readVal(SDM230_CURRENT_REVERSE_POWER_DEMAND);
  json[F("maxReverseDemand")]      = sdm.readVal(SDM230_MAXIMUM_REVERSE_POWER_DEMAND);
  json[F("currentDemand")]         = sdm.readVal(SDM230_CURRENT_DEMAND);
  json[F("maxCurrentDemand")]      = sdm.readVal(SDM230_MAXIMUM_CURRENT_DEMAND);
  json[F("totalEnergy")]           = sdm.readVal(SDM230_TOTAL_ACTIVE_ENERGY);
  json[F("totalReactiveEnergy")]   = sdm.readVal(SDM230_TOTAL_REACTIVE_ENERGY);
  json[F("parcialEnergy")]         = sdm.readVal(SDM230_CURRENT_RESETTABLE_TOTAL_ACTIVE_ENERGY);
  json[F("parcialReactiveEnergy")] = sdm.readVal(SDM230_CURRENT_RESETTABLE_TOTAL_REACTIVE_ENERGY);

  String result;
  if (serializeJson(json, result) == 0) {
    Serial.println(F("Failed generate json"));
  }
  String t = "node/"+String(nodeid)+"/"+"info";
  client.publish(t.c_str(),result.c_str());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //......
  readSDM230();
  delay(1000);                                                                  //wait a while before next loop
}
