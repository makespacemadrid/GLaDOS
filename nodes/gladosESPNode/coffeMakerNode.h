#ifndef COFFEEMAKER_H
#define COFFEEMAKER_H

#include "gladosMQTTNode.h"
#include "relay.h"

class coffeMakerNode : public gladosMQTTNode
{
enum powerStatus
{
	On,
	Off,
	PowerOffRequest
};

public:
	coffeMakerNode(String nodeID, String mqttServer, int port = 1883) : gladosMQTTNode(nodeID,mqttServer,port),
	coffeeSwitch(D0), ledStrip(D7,50,0.3), statusLeds(&ledStrip),m_leds(&ledStrip) ,tempSensor(A0),	relay1(D6),relay2(D3)
	{
		m_leds.addLed  (3,46);
		String str = "node/"+m_nodeID+"/";
		coffeeSwitch.setTopicPath(str);

		m_components.push_back(&ledStrip);
		m_components.push_back(&m_leds);
		m_components.push_back(&coffeeSwitch);
		m_components.push_back(&statusLeds);
		m_components.push_back(&relay1);
		m_components.push_back(&relay2);

		makingCoffe = false;
		heating     = false;
		coffeTimeout = 0;
		debounceTimer = 0;
	}

	void setupNode()
	{
			heat();
	}
	
	void updateNode()
	{
		if((debounceTimer <=0) && coffeeSwitch.statusChanged())
		{
			debounceTimer = 500;
			if(coffeeSwitch.status())
			{
				if(heating && makingCoffe)
				{
					Serial.println("Stop!");
					stop();
				}
				else
				{
					Serial.println("MakingCoffe!");
					makeCoffe();
				}
			}
		}

		if(coffeTimeout > 0)
		{
			coffeTimeout -= m_lastCicleTime;
			if(coffeTimeout <= 0)
			{
				stop();
			}
		}

		if(heating && !makingCoffe)
		{
			powerTimeout -= m_lastCicleTime;
			if(powerTimeout <= 0)
				off();
		}


		debounceTimer -= m_lastCicleTime;
		
            Serial.print("Heating: ");
            Serial.println(tempSensor.readRaw());		
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
		statusLeds.setColor(0,200,00);
		statusLeds.fade();
        m_leds.fade();
	}

	virtual void globalPowerOff()     	 
	{		
		statusLeds.setColor(200,0,0);
		m_leds.setColor(150,0,0);
		statusLeds.fade();
		m_leds.fade();
		off();
	}
	
	virtual void globalPowerOffRequest() 
	{
		statusLeds.setColor(200,200,0);
		statusLeds.glow();
        m_leds.setColor(100,100,0);
        m_leds.glow();
	}
	
	void makeCoffe()
	{
		heat();
        makingCoffe = true;
        relay2.activate();

        coffeTimeout = 30000;
        m_leds.setColor(0,100,0);
        m_leds.glow();
	}

	void stop()
	{
        m_leds.setColor(100,0,0);
        m_leds.fade();
		makingCoffe = false;
		relay2.deactivate();
	}
	
	void off()
	{
		heating      = false;
		makingCoffe  = false;
		relay1.deactivate();
		relay2.deactivate();
	}
	
	bool isHot()
	{
		return tempSensor.readRaw() <= 150;
	}
	
	void heat()
	{
        m_leds.setColor(150,0,0);
        m_leds.glow();
        relay1.activate();
        heating = true;

        while(!isHot() && heating)
        {
			relay1.activate();
            update();
            Serial.print("Heating: ");
            Serial.println(tempSensor.readRaw());
        }
        powerTimeout = 60000;
	}
	
protected:
    ntc100kTemperatureSensor    tempSensor;	
	button 					    coffeeSwitch;

	ws2812Strip 	ledStrip;
	ledStatusTrio 	statusLeds;
	ledBar          m_leds;	
	relay           relay1;
	relay			relay2;

	bool			makingCoffe;
	bool			heating;
	int				coffeTimeout;
	int				powerTimeout;
	powerStatus		m_powerStatus;
	int				debounceTimer;

};

#endif
