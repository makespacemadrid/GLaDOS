#ifndef NODE
#define NODE

#include <vector>
#include "spifsstorage.h"

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
	gladosMQTTNode() : mqttClient(espClient)
	{
		m_nodeID 	= storage.readConfig("nodeID"),
		m_server 	= storage.readConfig("overmind") ,
		m_port 		= storage.readConfig("overmindPort").toInt();

	  m_lastHeartbeat = millis(), m_lastConnect = 0, m_lastCicleTime = 0,	m_avgCicleTime = 0;
		m_cicleDelay = 0; m_reconnectTimer = 10000;
	}

	void setup()
	{
		Serial.print("Configuring node... ID:"); Serial.println(m_nodeID);

		Serial.println("Init Components...");
		yield();
		for(int i = 0 ; i < m_components.size() ; i++)
		{
			m_components[i]->setup();
		}
		setupNode();
#ifndef ESP32
	  //WiFi.begin("JarvisNetwork", "jointheovermind");

if(storage.readConfig("configMode") == "onReset")
{
	String be = storage.readConfig("bootError");
	m_bootError = be.toInt();
	m_bootError++;
	storage.writeConfig("bootError", String(m_bootError));
	Serial.print("Boot errors: ");Serial.println(m_bootError);
	if(m_bootError > storage.readConfig("maxFailBootConfig").toInt())
	{
		launchConfigPortal();
	}
}

		m_lastWifiStatus  = 0;
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
#endif
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
		int wifiStatus = WiFi.status();
		if(wifiStatus == 3)
		{
			if(m_lastWifiStatus != wifiStatus)
			{
				m_lastWifiStatus = wifiStatus;
				wifiConnected();
				if(m_bootError > 0)
				{
					m_bootError = 0;
					storage.writeConfig("bootError", "0");
				}
			}
			if (!mqttClient.connected())
			{
				reconnect();
			}
		}
		else
		{
			m_lastWifiStatus = wifiStatus;
			wifiDisconnected();
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
			}
			mqttClient.loop();
			yield();
		}

		updateNode();
#ifndef ESP32
		updateServer.handleClient();
		fileServer.handleClient();
#endif
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

		m_reconnectTimer -= m_lastCicleTime;
		if(m_reconnectTimer >0)
			return;
		Serial.println("Reconectando....");
		if (mqttClient.connect(m_nodeID.c_str())) {
			Serial.println("Connected!");
			// OK! Wait a second before continuing
			m_reconnectTimer = 0;
			delay(1000);
			followTopics();
			String str = "node/"+m_nodeID+"/system/status";
			mqttClient.publish(str.c_str(), "connected", false);
			serverConnected();
		}
		else
		{
			Serial.println("Cant connect!!");
			m_reconnectTimer = 30000;
			serverDisconnected();
		}
	}


	void imAlive()
	{
		Serial.print("Im alive! - ID:");Serial.print(m_nodeID);Serial.print("- IP:");Serial.println(localIP());
		Serial.print("avgCicleTimeMS:");Serial.print(m_lastCicleTime);Serial.print(" - lastcCicleTimeMS:"); Serial.print(m_lastCicleTime); Serial.print(" reconnectTimer::");Serial.println(m_reconnectTimer);
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

	void launchConfigPortal()
	{
		m_bootError = 0;
		storage.writeConfig("bootError", "0");
		Serial.println("Launching WifiManager..");
		wifiConfigMode();
		WiFiManager wifiManager;
		wifiManager.setConfigPortalTimeout(120);
		WiFiManagerParameter custom_nodeID("nodeID", "node_id",storage.readConfig("nodeID").c_str(), 40);
		WiFiManagerParameter custom_nodeType("nodeType", "node_type",storage.readConfig("nodeType").c_str(), 40);
		WiFiManagerParameter custom_mqtt_server("server", "mqtt_server",storage.readConfig("overmind").c_str(), 40);
		WiFiManagerParameter custom_mqtt_port("port", "mqtt_port",storage.readConfig("overmindPort").c_str(), 40);

		wifiManager.addParameter(&custom_nodeID);
		wifiManager.addParameter(&custom_nodeType);
		wifiManager.addParameter(&custom_mqtt_server);
		wifiManager.addParameter(&custom_mqtt_port);

		if (!wifiManager.startConfigPortal(m_nodeID.c_str(), "configureme")) {
			 Serial.println("failed to connect and hit timeout");
			 delay(3000);
			 //reset and try again, or maybe put it to deep sleep
			 ESP.reset();
			 delay(5000);
		}
		storage.writeConfig("nodeID",custom_nodeID.getValue());
		storage.writeConfig("nodeType",custom_nodeType.getValue());
		storage.writeConfig("overmind",custom_mqtt_server.getValue());
		storage.writeConfig("overmindPort",custom_mqtt_port.getValue());

		Serial.print("ID:");Serial.println(custom_nodeID.getValue());
		Serial.print("type:");Serial.println(custom_nodeType.getValue());
		Serial.print("mqtt host:");Serial.println(custom_mqtt_server.getValue());
		Serial.print("mqtt port:");Serial.println(custom_mqtt_port.getValue());
		yield();
		delay(3000);
		//reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(5000);
	}

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

#ifdef ESP32
	ESP32Storage storage
#else
	SPIFSStorage storage;
#endif

	String   m_nodeID,m_server;
	int			 m_port;

	long		  m_lastHeartbeat;
	long		  m_lastConnect;
	long		  m_lastCicleTime;
	long 		  m_reconnectTimer;
	float_t		m_avgCicleTime;
	int			  m_cicleDelay;
	int				m_lastWifiStatus;
	int 			m_bootError;

	std::vector<nodeComponent*> m_components;

	void followTopics(){
		mqttClient.subscribe("space/powerStatus");

		followTopicsNode();
	}

	virtual void followTopicsNode() {return;}
};

#endif
