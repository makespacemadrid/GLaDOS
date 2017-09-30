#ifndef NODE
#define NODE

#include <vector>


struct mqttPublication
{
	String path  = "";
	String topic = "";
	String val   = "";
	bool persist = false;
	
	String getTopic()
	{
		return path+topic;
	}
};

class nodeComponent
{
public:
	nodeComponent(String id = "Comp")
	{
		m_pollTimer = 10000;
		m_lastPoll  = millis();
		m_id = id;
	}

	void setTopicPath(String path) {m_topicPath = path;}
	
	std::vector<mqttPublication> getPublications()
	{
		std::vector<mqttPublication> result = m_publications;
		m_publications.clear();
		return result;
	}

	void setPollInterval(int interval)
	{
		m_pollTimer = interval;
	}

	void update()  
	{
		long now = millis();
		if( (now - m_lastPoll) > m_pollTimer )
		{
			m_lastPoll = now;
			readComponent();
		}

		updateComponent();
	}
		
	virtual void setup()   			{return;}
	virtual void updateComponent() 	{return;}
	virtual void readComponent()	{return;}

	
protected:
	String						  m_id;
	String  					  m_topicPath;
	std::vector<mqttPublication>  m_publications;

	int  m_pollTimer;
	long m_lastPoll;
};


class gladosMQTTNode
{
public:
	gladosMQTTNode(String nodeID, String mqttServer, int port = 1883) : mqttClient(espClient)
	{
		m_nodeID = nodeID, m_server = mqttServer , m_port = port;

	    m_lastHeartbeat = millis(), m_lastConnect = 0, m_lastCicleTime = 0,	m_avgCicleTime = 0;
		m_cicleDelay = 0;
	}

	void setup()
	{
		Serial.print("Configuring node... ID:"); Serial.print(m_nodeID);
		Serial.println("Init Components...");
		yield();
		for(int i = 0 ; i < m_components.size() ; i++)
		{
			m_components[i]->setup();
		}
		setupNode();

		Serial.println("Launching WifiManager..");
		yield();
		wifiDisconnected();
		WiFiManager wifiManager;
		//exit after config instead of connecting
		wifiManager.setBreakAfterConfig(true);
    wifiManager.setTimeout(30);
		//reset settings - for testing
		//wifiManager.resetSettings();
		//tries to connect to last known settings
		//if it does not connect it starts an access point with the specified name
		//here  "AutoConnectAP" with password "password"
		//and goes into a blocking loop awaiting configuration
		while (!wifiManager.autoConnect(m_nodeID.c_str(), "configureme")) {
			Serial.print("Cannot connect wireless. Config network: "); Serial.print(m_nodeID); Serial.print(" / "); Serial.println("configureme");
			wifiConfigMode();
			//delay(3000);
		  //ESP.reset();
			//delay(5000);
		}
		
		wifiConnected();
		serverDisconnected();
		if(MDNS.begin(m_nodeID.c_str()))
    {
            Serial.print("MDNS started, name:");Serial.print(m_nodeID);Serial.println(".local");
		}
		else
		{
			Serial.print("Can't start MDNS");
		}
		startHttpUpdater(8080);
		startHttpFileServer(80);
		
		Serial.print("Configuring MQTT server:"); Serial.print(m_server);
		Serial.print(" port: "); Serial.print(m_port);
		Serial.println(".");
		yield();
				
		mqttClient.setServer(m_server.c_str(), m_port);
		Serial.println("Setup Completed!");
	}
	
	virtual void setupNode() {return;}


	void update()
	{
		long now = millis();
		
		if (!mqttClient.connected()) {
			serverDisconnected();
			reconnect();
		}

		mqttClient.loop();
		
		for(int i = 0 ; i < m_components.size() ; i++)
		{
			m_components[i]->update();
			
			std::vector<mqttPublication> pubs = m_components[i]->getPublications();
			for(int p = 0 ; p < pubs.size() ; p++)
			{
				Serial.print("pub:");Serial.print(pubs[p].getTopic());Serial.println(pubs[p].val);
				mqttClient.publish(pubs[p].getTopic().c_str(),pubs[p].val.c_str(),pubs[p].persist);
				mqttClient.loop();
				yield();
			}
		}

		updateNode();
		
		if (now - m_lastHeartbeat > 30000) {
			m_lastHeartbeat = now;
			imAlive();
		}
		
		if(m_cicleDelay)
			delay(m_cicleDelay);
		
		yield();
		m_lastCicleTime = millis() - now;
		m_avgCicleTime = (m_avgCicleTime + m_lastCicleTime) / 2.0f;
	}
	
	virtual void updateNode() {return;}

		

	void reconnect() {
		
		Serial.println("Reconectando....");
		if (mqttClient.connect(m_nodeID.c_str())) {
			// OK! Wait a second before continuing
			delay(1000);
			serverConnected();
			followTopics();
			String str = "node/"+m_nodeID+"/system/status";
			mqttClient.publish(str.c_str(), "connected", false);      
		}
   else
   {
        serverDisconnected();
   }
	}
	

	void imAlive()
	{
		Serial.print("Im alive! - ID:");Serial.print(m_nodeID);Serial.print("- IP:");Serial.println(localIP());
		Serial.print("avgCicleTimeMS:");Serial.print(m_lastCicleTime);Serial.print(" - lastcCicleTimeMS:"); Serial.println(m_lastCicleTime);
		Serial.println("");
		
		String topic = "node/"+m_nodeID+"/system/status";
		String val   = "alive" ;
		mqttClient.publish(topic.c_str(),val.c_str(),false);

		topic = "node/"+m_nodeID+"/system/avgCicleTime";
		val   = String(m_lastCicleTime);
		mqttClient.publish(topic.c_str(),val.c_str(),false);
		
		topic = "node/"+m_nodeID+"/system/lastCicleTime";
		val = String(m_lastCicleTime);
		mqttClient.publish(topic.c_str(),val.c_str(),false);
	}

	PubSubClient& MQTTClient() {return mqttClient;}


	void processTopic(char* topic, byte* payload, unsigned int length)
	{
		String top = topic;
		String val;

		Serial.print("Topic: ");
		Serial.print(top);  
		Serial.print("PL: ");
		Serial.print(length);

		for(int i = 0 ; i < length ; i++)
		{
			val += (char)payload[i];
		}
		
		Serial.print(" - payload: ");
		Serial.println(val);
		
		if(top == "space/powerStatus")
		{
			if(val == "on")
			{
				m_cicleDelay = 0;
				globalPowerOn();
			}
			else if(val == "off")
			{
				globalPowerOff();
				m_cicleDelay = 20;
			}
			else if(val == "powerOffRequest")
				globalPowerOffRequest();
		}
		else
			processTopicNode(top,val);
	}


    String localIP()
    {
      IPAddress ip;
      ip = WiFi.localIP();

      String result;
      result += ip[0];
      result += ".";
      result += ip[1];
      result += ".";
      result += ip[2];
      result += ".";
      result += ip[3];
      return result;
    }
	
	virtual void processTopicNode(String& topic,String& Val)   
	{ Serial.print("W:processTopicNode()->Unknown topic or overload me! :");Serial.println(topic);}
	
	virtual void wifiConnected()      {return;}
	virtual void wifiDisconnected()   {return;}
	virtual void wifiConfigMode()	  {return;}
	virtual void serverConnected()    {return;}
	virtual void serverDisconnected() {return;}
	
	virtual void globalPowerOn()	  	 {return;}
	virtual void globalPowerOff()     	 {return;}
	virtual void globalPowerOffRequest() {return;}
	
protected:
	WiFiClient   espClient;
	PubSubClient mqttClient;
	
	String       m_nodeID,m_server;
	int			 m_port;

	long    	m_lastHeartbeat;
	long	 	m_lastConnect;
	long		m_lastCicleTime;
	float_t		m_avgCicleTime;
	int			m_cicleDelay;
	
	std::vector<nodeComponent*> m_components;

	void followTopics(){
		mqttClient.subscribe("space/powerStatus");

		followTopicsNode();
	}
	
	virtual void followTopicsNode() {return;}
};

#endif
