#ifndef LMODULE
#define LMODULE

#include "gladosMQTTNode.h"
#include "relay.h"

class lightControl : public gladosMQTTNode
{
enum powerStatus
{
	On,
	Off,
	PowerOffRequest
};

public:
	lightControl(String nodeID, String mqttServer, int port = 1883) : gladosMQTTNode(nodeID,mqttServer,port),
	ledStrip(D7,15,0.65), statusLeds(&ledStrip),
	relay1(D2),relay2(D1),relay3(D8)
	{

		String str = "node/"+m_nodeID+"/";
		mainSwitch.setTopicPath(str);

		m_components.push_back(&ledStrip);
		m_components.push_back(&statusLeds);
		m_components.push_back(&relay1);
		m_components.push_back(&relay2);
		m_components.push_back(&relay3);
		
		debounceTimer = 0;
		courtesyTimer = 0;
	}

	void setupNode()
	{
		
	}
	
	void updateNode()
	{

		if((debounceTimer <=0) && mainSwitch.statusChanged())
		{
			debounceTimer = 50;
			if(mainSwitch.status())
			{
				
			}
		}

		if(courtesyTimer > 0)
		{
			courtesyTimer -= m_lastCicleTime;
			if(courtesyTimer <= 0)
			{
				relay3.deactivate();
			}
		}
		debounceTimer -= m_lastCicleTime;
	}

	virtual void wifiConnected()      {return;}

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

	}	
	

	virtual void globalPowerOn()	  	 
	{
		courtesyTimer = -1;
		relay1.activate();
		relay2.activate();
		relay3.deactivate();
		statusLeds.setColor(0,200,00);
		statusLeds.fade();
	}
	virtual void globalPowerOff()     	 
	{
		relay3.activate();
		courtesyTimer = 35000;
		delay(200);
		relay1.deactivate();
		relay2.deactivate();
		
		statusLeds.setColor(200,0,0);
		statusLeds.fade();
	}
	
	virtual void globalPowerOffRequest() 
	{
		relay3.activate();
		statusLeds.setColor(200,200,0);
		statusLeds.glow();
	}
	
protected:
	
	button 					mainSwitch;

	ws2812Strip 	ledStrip;
	ledStatusTrio 	statusLeds;
	relay           relay1;
	relay			relay2;
	relay			relay3;

	powerStatus		m_powerStatus;
	long			powerOffDelay;
	int				debounceTimer;
	int             courtesyTimer;
};



#endif
