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
	coffeeSwitch(D0), ledStrip(D7,50,0.2), statusLeds(&ledStrip),leds(&ledStrip) ,tempSensor(A0),	heaterRelay(D6),pumpRelay(D3)
	{
		leds.addLed  (3,46);
		String str = "node/"+m_nodeID+"/";
		coffeeSwitch.setTopicPath(str);
		tempSensor.setTopicPath(str);


		m_components.push_back(&ledStrip);
		m_components.push_back(&leds);
		m_components.push_back(&coffeeSwitch);
		m_components.push_back(&statusLeds);
		m_components.push_back(&tempSensor);
		m_components.push_back(&heaterRelay);
		m_components.push_back(&pumpRelay);
		_makingCoffe = false;
		_heating     = false;
		_coffeTimeout = 0;
		_debounceTimer = 0;
	}

	void setupNode()
	{
		heaterRelay.activate();
		_heating = true;
		_powerTimeout = 120000;

		_coffeMade = storage.readConfig("coffeMade").toInt();
		Serial.print("Coffe made: ");Serial.println(_coffeMade);
	}

	void updateNode()
	{
		//Logica de hacer cafe/parar.
		if((_debounceTimer <=0) && coffeeSwitch.statusChanged())
		{
			_debounceTimer = 250;
			if(coffeeSwitch.status())
			{
				if(_heating && _makingCoffe)
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

		//Temporizador del cafe.
		if(_coffeTimeout > 0)
		{
			_coffeTimeout -= m_lastCicleTime;
			if(_coffeTimeout <= 0)
			{
				Serial.println("Coffe made!");
				stop();
				String path	  = "node/"+m_nodeID+"/coffeDone";
				mqttClient.publish(path.c_str(),"now",true);
			}
		}

		//Temporizador de apagado
		if(_heating && !_makingCoffe)
		{
			_powerTimeout -= m_lastCicleTime;
			if(_powerTimeout <= 0)
			{
				Serial.println("Power Timeout, off");
				off();
			}
		}

		//Reaccion de los leds
		if(_makingCoffe)
		{
			leds.setProgress( (1.0-(_coffeTimeout/24000.0))*100,0,0,150,true);
		}
		else if(_heating)
		{
			int progress = (tempSensor.readData()/60)*100;
			leds.setProgress(progress,150,0,0,true);
			String p = "node/"+m_nodeID+"/heating";
			mqttClient.publish(p.c_str(),String(progress).c_str(),true);
		}

		_debounceTimer -= m_lastCicleTime;
	}


	virtual void wifiConnected()      {return;}

	virtual void wifiDisconnected()
	{
		statusLeds.setColor(100,0,0);
	}

	virtual void wifiConfigMode()
	{
		statusLeds.setColor(100,0,100);
	}

	virtual void serverConnected()
	{
		statusLeds.setColor(0,150,0);
		statusLeds.fade();
	}

	virtual void serverDisconnected()
	{
		statusLeds.setColor(0,0,100);
	}

	virtual void globalPowerOn()
	{
		statusLeds.setColor(0,200,00);
		statusLeds.fade();
	}

	virtual void globalPowerOff()
	{
		statusLeds.setColor(200,0,0);
		statusLeds.fade();
		off();
	}

	virtual void globalPowerOffRequest()
	{
		statusLeds.setColor(200,200,0);
		statusLeds.glow();
	}

	void makeCoffe()
	{
		Serial.println("Making coffe, checking heat...");
		heat();
		String p = "node/"+m_nodeID+"/coffeDone";
		mqttClient.publish(p.c_str(),"now",true);
    _makingCoffe = true;
		_coffeMade++;
		storage.writeConfig("coffeMade", String(_coffeMade));
		p	= "node/"+m_nodeID+"/coffeMade";
		String val = String(_coffeMade);
		mqttClient.publish(p.c_str(),val.c_str(),true);
    pumpRelay.activate();
    _coffeTimeout = 24000;
		Serial.print("Activating Pump for");Serial.print(String(_coffeTimeout/1000));Serial.println("s");
    leds.setColor(0,100,0);
    leds.glow();
	}

	void stop()
	{
		leds.setColor(0,0,100);
		leds.fade();
		_makingCoffe = false;
		pumpRelay.deactivate();
	}

	void off()
	{
		_heating      = false;
		_makingCoffe  = false;
		heaterRelay.deactivate();
		pumpRelay.deactivate();
		leds.fade();
		statusLeds.fade();
	}

	bool isHot()
	{
		return tempSensor.readData() >= 60;
	}

	void heat()
	{
        heaterRelay.activate();
        _heating = true;
        _powerTimeout = 120000;
        while(!isHot() && _heating)
        {
            update();
            //Serial.print("Heating: ");
            //Serial.println(tempSensor.readRaw());
        }
	}

protected:
	ntc100k					tempSensor;
	button 					coffeeSwitch;
	ws2812Strip 		ledStrip;
	ledStatusTrio 	statusLeds;
	ledBar					leds;
	relay						heaterRelay;
	relay						pumpRelay;

	bool				_makingCoffe;
	bool				_heating;
	int					_coffeTimeout;
	int					_powerTimeout;
	int					_debounceTimer;
	uint16_t		_coffeMade;
	powerStatus	_powerStatus;

};

#endif
