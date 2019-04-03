#ifndef DOORCONTROL
#define DOORCONTROL

#include "gladosMQTTNode.h"
#include "nfcModule.h"
#include "relay.h"

#ifdef HARDCODED_CARDS
#include "cards.h"
#endif

class doorControlNode : public gladosMQTTNode
{
public:
  doorControlNode() : gladosMQTTNode(),
  ledStrip(D1,3,0.2), statusLeds(&ledStrip),doorRelay(D0)
  {
    m_components.push_back(&ledStrip);
		m_components.push_back(&statusLeds);
		m_components.push_back(&doorRelay);
		//m_components.push_back(&nfc);
    nfc.setup();
  }

  void setupNode()
	{
    statusLeds.setColor(50,50,00);
    statusLeds.glow();
    statusLeds.animate();
	}

	void updateNode()
	{
    String cardid = nfc.readCard();
    if(cardid != "0000000000000000")
    {
      Serial.print("rfid: "); Serial.println(cardid);
      statusLeds.setColor(0,0,100);
      statusLeds.animate();
      if(validateCard(cardid)>5)
      {
        statusLeds.setColor(0,255,250);
        doorRelay.activate();
        openTimer = 4000;
      }
      else if(validateCard(cardid)>1)
      {
        statusLeds.setColor(0,255,0);
        doorRelay.activate();
        openTimer = 4000;
      }
      else if(validateCard(cardid)>0)
      {
        statusLeds.setColor(150,150,0);
        doorRelay.activate();
        openTimer = 4000;
      }
      else
      {
        statusLeds.setColor(255,0,0);
        statusLeds.fade();
        statusLeds.animate();
      }
    }

    if(openTimer > 0)
    {
      openTimer -= m_lastCicleTime;
      if(openTimer <= 0)
      {
        doorRelay.deactivate();
        statusLeds.fade();
      }
    }

	}

	virtual void wifiConnected()
  {
    if(storage.updateUserAuth())
      statusLeds.instantBlink(0,0,150,2);
    else
      statusLeds.instantBlink(0,0,150,1);
  }

	virtual void wifiDisconnected()
  {
    statusLeds.instantBlink(50,50,0,1);
  }

	virtual void wifiConfigMode()
  {
    statusLeds.setColor(150,0,150);
    statusLeds.animate();
  }

	virtual void serverConnected()
  {
    statusLeds.instantBlink(0,150,0,2);
  }

	virtual void serverDisconnected()
  {
    statusLeds.instantBlink(100,0,0,2);
  }

	virtual void globalPowerOn()
	{
    statusLeds.instantBlink(0,150,0,1);
	}
	virtual void globalPowerOff()
	{
		statusLeds.setColor(200,0,0);
		statusLeds.fade();
	}

	virtual void globalPowerOffRequest()
	{
		statusLeds.setColor(200,200,0);
		statusLeds.glow();
	}

  int validateCard(String cardid)
  {
    String time = "00";
    if(WiFi.status()==3)
      time = ntpClient.getUnixTime();

    msmUserAccess msmUser = storage.userAuthLevel("sideDoor",cardid);
    Serial.print("Time: "+time);
    Serial.print("Cardid: "  + cardid);
    Serial.print(" - User: "  + msmUser.nickname);
    Serial.println(" - Level: " + String(msmUser.level));

    File logFile = SPIFFS.open("/sideDoorLog.log","a");
    if(logFile)
    {
      logFile.print(time + "\t" + cardid+" - ");
      logFile.println(msmUser.nickname + "\t\t- " + msmUser.level);
    }

    if(msmUser.level > 0)
    {
      String topic  = "node/"+m_nodeID+"/access";
      String val    = msmUser.nickname;
      mqttClient.publish(topic.c_str(),val.c_str(),false);
      return msmUser.level;
    }
// a partir de aqui es tag no registrado o error de la spiffs
    #ifdef HARDCODED_CARDS
    for(int i = 0 ; i < sizeof(harcodedCards) ; i++)
    {
      if(cardid == harcodedCards[i])
      {
        Serial.println("ADMIN CARD!");
        String topic = "node/"+m_nodeID+"/access";
        String val   = "admin-"+cardid;
        mqttClient.publish(topic.c_str(),val.c_str(),false);
        File logFile = SPIFFS.open("/sideDoorLog.log","a");
        if(logFile)
        {
          logFile.print(String(ntpClient.getUnixTime()) + "\t" + cardid);
          logFile.println("ADMIN-"+cardid+"\t\t- " + String(6));
        }
        return 6;
      }
    }
    #endif

    String topic  = "node/"+m_nodeID+"/access";
    String val    = "reject-"+msmUser.cardid;
    mqttClient.publish(topic.c_str(),val.c_str(),false);
    return msmUser.level;
  }

protected:
  ws2812Strip     ledStrip;
	ledStatusTrio   statusLeds;
  relay           doorRelay;
  nfcModuleMFRC522 nfc;

  int             openTimer;

};

#endif
