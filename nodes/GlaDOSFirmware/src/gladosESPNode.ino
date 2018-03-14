#include <FS.h>                   //this needs to be first, or it all crashes and burns...

#ifdef ESP8266
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DNSServer.h>
#include "espweb.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#elif ESP32
#include <WiFi.h>
#define D0 0
#define D1 0
#define D2 0
#define D3 0
#define D4 0
#define D5 0
#define D6 0
#define D7 0
#define D8 0
#endif

#include <ArduinoJson.h>

#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <vector>
#include <MFRC522.h>


#include "gladosMQTTNode.h"
#include "makeSwitchNode.h"
#include "lightControlNode.h"
#include "coffeMakerNode.h"
#include "mqttLamp.h"
#include "machineAccess.h"
#include "doorControlNode.h"

doorControlNode node("sideDoor","192.168.10.10");
//coffeMakerNode node("coffeMaker0", "192.168.10.10");
//makeSwitchNode node("makeSwitch","192.168.10.10");
//lightControl node("lightControl","192.168.10.10");


void subCallback(char* topic, byte* payload, unsigned int length)
{
	node.processTopic(topic,payload,length);
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(100);
  yield();
	Serial.print("...\n");
  node.setup();
  node.MQTTClient().setCallback(subCallback);
}

int i = 0 ;
void loop() {
  node.update();
}
