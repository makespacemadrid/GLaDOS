#ifndef DOORCONTROL
#define DOORCONTROL

#include "gladosMQTTNode.h"
#include "nfcModule.h"
#include "relay.h"

class doorControlNode : public gladosMQTTNode
{
public:
  doorControlNode(String nodeID, String mqttServer, int port = 1883) : gladosMQTTNode(nodeID,mqttServer,port),
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
    statusLeds.setColor(50,50,0);
    statusLeds.animate();
	}

	void updateNode()
	{
    String cardid= nfc.readCard();
    //Serial.print("rfid: "); Serial.println(cardid);
	}

	virtual void wifiConnected()
  {
    storage.updateUserAuth();
    statusLeds.setColor(0,0,200);
    statusLeds.glow();
    statusLeds.animate();

  }

	virtual void wifiDisconnected()    {}

	virtual void wifiConfigMode()
  {
    statusLeds.setColor(200,0,200);
    statusLeds.animate();
  }

	virtual void serverConnected()
  {
    statusLeds.setColor(200,0,0);
    statusLeds.fade();
    statusLeds.animate();
  }

	virtual void serverDisconnected()  {}

	virtual void globalPowerOn()
	{
		statusLeds.setColor(0,200,00);
		statusLeds.fade();
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

protected:
  ws2812Strip     ledStrip;
	ledStatusTrio   statusLeds;
  relay           doorRelay;
  nfcModuleMFRC522 nfc;

};

#endif
