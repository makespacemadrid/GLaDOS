#ifndef LAMPB
#define LAMPB
#include "gladosMQTTNode.h"

class lightButton : public gladosMQTTNode
{
public:
  lightButton() : gladosMQTTNode(),
  ledStrip(D6,30,0.6),leds(&ledStrip),pushButton(D2),
  centerLeds(&ledStrip),topLeds(&ledStrip),rigthLeds(&ledStrip),bottomLeds(&ledStrip),leftLeds(&ledStrip)
  {

    String str = "node/"+m_nodeID+"/";
    leds.addLed  (0,25);
    centerLeds.addLed(0,6);
    topLeds.addLed(6,5);
    rigthLeds.addLed(11,4);
    bottomLeds.addLed(15,5);
    leftLeds.addLed(21,4);
    m_components.push_back(&ledStrip);
    m_components.push_back(&leds);
    m_components.push_back(&pushButton);
    m_cicleDelay = 5;
  }

  void setupNode()
  {
    centerLeds.setColor(0,250,0);
    ledStrip.update();
    delay(250);
    topLeds.setColor(250,0,0);
    bottomLeds.setColor(250,0,0);
    ledStrip.update();
    delay(250);
    rigthLeds.setColor(0,0,255);
    leftLeds.setColor(0,0,255);
    ledStrip.update();
    delay(250);
    centerLeds.setColor(0,250,0);
    ledStrip.update();
    delay(250);
    topLeds.setColor(250,0,0);
    bottomLeds.setColor(250,0,0);
    ledStrip.update();
    delay(250);
    rigthLeds.setColor(0,0,255);
    leftLeds.setColor(0,0,255);
    ledStrip.update();
    delay(250);

    centerLeds.setColor(0,0,255);
    ledStrip.update();
    delay(250);
    topLeds.setColor(250,255,0);
    bottomLeds.setColor(250,255,0);
    ledStrip.update();
    delay(250);
    rigthLeds.setColor(0,0,255);
    leftLeds.setColor(0,0,255);
    ledStrip.update();
    delay(250);

    centerLeds.setColor(0,250,250);
    ledStrip.update();
    delay(250);
    topLeds.setColor(250,0,250);
    bottomLeds.setColor(250,0,250);
    ledStrip.update();
    delay(250);
    rigthLeds.setColor(250,0,255);
    leftLeds.setColor(250,0,255);
    ledStrip.update();
    delay(250);
    leds.instantFade();
    leds.setColor(150,0,0);
    leds.glow();
  }

  void normalLight()
  {
    leds.instantFade();
    leds.baseColor();
    ledStrip.update();
  }

  void pressLight()
  {
    leds.setColor(0,255,0);
    ledStrip.update();
  }

  void updateNode()
  {
    if(pushButton.statusChanged())
      if(pushButton.status())
      {
        String str = "node/"+m_nodeID+"/status";
        mqttClient.publish(str.c_str() ,String("press").c_str(),false);
        str = "node/"+m_nodeID+"/press";
        mqttClient.publish(str.c_str() ,String("1").c_str(),false);
        mqttClient.loop();
        pressLight();
        int pressTime = 0;
        while(pushButton.status())
        {
          delay(5);
          pushButton.update();
          pressTime+=5;
        }
        str = "node/"+m_nodeID+"/pressTime";
        mqttClient.publish(str.c_str() ,String(pressTime).c_str(),false);
        str = "node/"+m_nodeID+"/status";
        mqttClient.publish(str.c_str() ,String("release").c_str(),false);
        mqttClient.loop();
        normalLight();
      }

  }

  void followTopicsNode()
  {
    mqttClient.subscribe("node/Shift/randomColor");
    mqttClient.subscribe("node/Shift/color");
    mqttClient.subscribe("node/Shift/baseColor");
    mqttClient.subscribe("node/Shift/animation");
    mqttClient.subscribe("node/Shift/brightness");
  }


  void processTopicNode(String& topic,String& Val)
  {
    if(topic == "node/lamp/x")
    {
      int ival = Val.toInt();
      if(ival >100)
        ival = 100;

      ledStrip.setBrightness(ival);
    } else if(topic =="node/Shift/randomColor")
    {
      leds.randomColor();
    }
    else if(topic =="node/Shift/color")
    {
      StaticJsonBuffer<100>  data;
      JsonObject& c = data.parseObject(Val);
      Serial.println(Val);
      if (!c.success())
      {
        Serial.println("Parsing json failed !");
        return;
      }
      String r = c["r"]; int cr = r.toInt();
      String g = c["g"]; int cg = g.toInt();
      String b = c["b"]; int cb = b.toInt();
      leds.setColor(cr,cg,cb);

    }
    else if(topic =="node/Shift/baseColor")
    {
      StaticJsonBuffer<100>  data;
      JsonObject& c = data.parseObject(Val);
      Serial.println(Val);
      if (!c.success())
      {
        Serial.println("Parsing json failed !");
        return;
      }
      String r = c["r"]; int cr = r.toInt();
      String g = c["g"]; int cg = g.toInt();
      String b = c["b"]; int cb = b.toInt();
      leds.setBaseColor(cr,cg,cb);
      leds.baseColor();
    }
    else if(topic =="node/Shift/animation")
    {
      leds.setAnimationJSON(Val);
    }
    else if(topic =="node/Shift/brightness")
    {
      ledStrip.setBrightness(int(Val.toInt()));
    }

  }

  virtual void wifiConnected()
  {
    leds.instantFade();
    leds.setColor(100,100,0);
    leds.glow();
  }

  virtual void wifiDisconnected()
  {
    leds.instantFade();
    leds.setColor(100,0,0);
    leds.glow();
  }

  virtual void wifiConfigMode()
  {
    leds.instantFade();
    leds.setColor(0,150,150);
    leds.glow();
  }

  virtual void serverConnected()
  {
    normalLight();
  }

  virtual void serverDisconnected()
  {
    leds.instantFade();
    leds.setColor(0,0,150);
    leds.glow();
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
  ledBar				  centerLeds;
  ledBar				  topLeds;
  ledBar				  rigthLeds;
  ledBar				  leftLeds;
  ledBar				  bottomLeds;
  button          pushButton;

};

#endif
