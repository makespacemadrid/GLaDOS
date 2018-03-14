#ifndef MAKESNODE
#define MAKESNODE

#include "gladosMQTTNode.h"
#include "ws2812led.h"
#include "buttons.h"
#include "temperatureSensor.h"
#include "piezoSpeaker.h"

enum powerStatus
{
	On,
	Off,
	PowerOffRequest
};

class makeSwitchNode : public gladosMQTTNode
{

public:
	makeSwitchNode(String nodeID, String mqttServer, int port = 1883) : gladosMQTTNode(nodeID,mqttServer,port),
	mainSwitch(D0), speaker(D2) , dhtSensor(D1,DHT22) ,
	ledStrip(D3,15,0.65), statusLeds(&ledStrip), onLeds(&ledStrip) , offLeds(&ledStrip)
	{
		onLeds.addLed  (3,6);
		offLeds.addLed (9,6);

		String str = "node/"+m_nodeID+"/";
		mainSwitch.setTopicPath(str);
		dhtSensor.setTopicPath(str);

		m_components.push_back(&mainSwitch);
		m_components.push_back(&speaker);
		m_components.push_back(&dhtSensor);

		m_components.push_back(&ledStrip);
		m_components.push_back(&statusLeds);
		m_components.push_back(&onLeds);
		m_components.push_back(&offLeds);
		
		debounceTimer = 0;
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
				if(m_powerStatus == PowerOffRequest)
				{
					m_powerStatus = On;
					mqttClient.publish("space/powerStatus", "on",true);
					speaker.playTone(100,50);
					delay(25);
					speaker.playTone(150,50);
					
				}
				else
				{
					m_powerStatus = On;
					mqttClient.publish("space/powerStatus", "on",true);
					mqttClient.publish("space/status", "Open",true);
					speaker.playRtttl("PacMan:d=16,o=6,b=140:32d#,32e,f,32f,32f#,g,32g,32g#,a,8b");
				}
			}
			else
			{
				mqttClient.publish("space/powerStatus", "powerOffRequest",true);
				m_powerStatus = PowerOffRequest;
				powerOffDelay = 10500;
				beepTimer	  = 1000;
			}
		}

		if(m_powerStatus == PowerOffRequest)
		{
			powerOffDelay -= m_lastCicleTime;
			beepTimer     -= m_lastCicleTime;
			
			if(beepTimer <= 0)
			{
				speaker.playTone(50,100);
				beepTimer = 1000;
			}
			
			if(powerOffDelay <= 0)
			{
				m_powerStatus = Off;
				mqttClient.publish("space/powerStatus", "off",true);
				mqttClient.publish("space/status", "Close",true);
			}
		}
		debounceTimer -= m_lastCicleTime;
	}

	virtual void wifiConnected()      {return;}

	virtual void wifiDisconnected()   
	{
		onLeds.setColor (200,0,0);
		offLeds.setColor(200,0,0);
	}

	virtual void wifiConfigMode()	  
	{
		onLeds.setColor (150,150,0);
		offLeds.setColor(150,150,0);
	}

	virtual void serverConnected()    
	{
    if(mainSwitch.status())
      ledStatusOpen();
    else
      ledStatusClosed();
	}

	virtual void serverDisconnected() 
	{
		onLeds.setColor (0,0,200);
		offLeds.setColor(0,0,200);
	}	
	
	void ledStatusOpen()
	{
		onLeds.setColor (0,200,0);
		offLeds.off();		
	}
	
	void ledStatusClosed()
	{
		onLeds.setColor (0,0,250);
		offLeds.setColor(100,0,0);
		onLeds.glow();		
	}

	void ledStatusPowerOffRequest()
	{
		speaker.playTone(50,250);
		onLeds.setColor (200,200,0);
		offLeds.setColor(200,0,0);
		onLeds.glow();		
	}

	virtual void globalPowerOn()	  	 
	{
		statusLeds.setColor(0,200,00);
		statusLeds.fade();
		ledStatusOpen();
	}
	virtual void globalPowerOff()     	 
	{
		statusLeds.setColor(200,0,0);
		statusLeds.fade();
		ledStatusClosed();
		speaker.playRtttl("dead:d=4,o=4,b=300:4b4,4f5,4p,4f5,3f5,3e5,3d5,4c5");		
	}
	
	virtual void globalPowerOffRequest() 
	{
		ledStatusPowerOffRequest();
		statusLeds.setColor(200,200,0);
		statusLeds.glow();
	}
	
protected:
	
	button 					mainSwitch;
	piezoSpeaker 			speaker;
	dhtTemperatureSensor 	dhtSensor;

	ws2812Strip 	ledStrip;
	ledStatusTrio 	statusLeds;
	ledBar 			onLeds;
	ledBar 			offLeds;

	powerStatus		m_powerStatus;
	long			powerOffDelay;
	long			beepTimer;
	int				debounceTimer;
};

#endif
