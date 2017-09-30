#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <Adafruit_NeoPixel.h>

#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <vector>

#include "espweb.h"
#include "gladosMQTTNode.h"
#include "makeSwitchNode.h"
#include "lightControlNode.h"
#include "coffeMakerNode.h"



coffeMakerNode node("coffeMaker", "192.168.10.10");
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

  node.setup();
  node.MQTTClient().setCallback(subCallback);
}


void loop() {
  
  node.update();
  
}
