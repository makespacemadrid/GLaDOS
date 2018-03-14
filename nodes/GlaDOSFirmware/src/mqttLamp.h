#ifndef LAMP
#define LAMP
#include "gladosMQTTNode.h"

class mqttLamp : public gladosMQTTNode
{
public:
  mqttLamp(String nodeID, String mqttServer, int port = 1883) : gladosMQTTNode(nodeID,mqttServer,port),
  ledStrip(D4,100,0.8),leds(&ledStrip)
  {
    String str = "node/"+m_nodeID+"/";
    leds.addLed  (0,100);
    m_components.push_back(&ledStrip);
    m_components.push_back(&leds);
    m_cicleDelay = 15;
  }

  void setupNode()
  {
    leds.rainbow();
  }

  void updateNode()
  {

  }

  void followTopicsNode()
  {
    //mqttClient.subscribe("node/coffeMaker0/Finished");
    //mqttClient.subscribe("node/coffeMaker0/makingCoffe");
    //mqttClient.subscribe("node/coffeMaker0/heating");
    mqttClient.subscribe("node/lamp/x");
    mqttClient.subscribe("node/lamp/y");
  }

  void processTopicNode(String& topic,String& Val)
  {
    if(topic == "node/lamp/x")
    {
      int ival = Val.toInt();
      if(ival >100)
        ival = 100;

      ledStrip.setBrightness(ival);
    } else if(topic == "node/lamp/y")
    {
      int ival = Val.toInt();
      leds.wheel(ival);
    }

  }

  virtual void wifiConnected()
  {
    //leds.setColor(0,100,0);
  }

  virtual void wifiDisconnected()
  {

  }

  virtual void wifiConfigMode()
  {

  }

  virtual void serverConnected()
  {

  }

  virtual void serverDisconnected()
  {
    //leds.setColor(0,0,100);
  }

  virtual void globalPowerOn()
  {
    leds.rainbow();
  }

  virtual void globalPowerOff()
  {
    leds.off();
  }

  virtual void globalPowerOffRequest()
  {
    leds.setColor(200,200,0);
    leds.glow();
  }

protected:
  ws2812Strip 		ledStrip;
  ledBar					leds;

};

#endif
