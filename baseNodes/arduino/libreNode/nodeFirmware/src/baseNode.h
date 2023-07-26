#ifndef BASENODE
#define BASENODE


//Librerias
#include "extra.h"
#include "storage.h"
#include "ledController.h"
#include "nodeComponent.h"
#include "ledGadget.h"

//

extern void ledsOff();
extern void ledsOTAMode();
extern void ledsOTAEnd();
extern void ledsOtaProgress(uint8_t p);
extern void ledsOTAError();
extern void mqttCallback(char* topic, byte* payload, unsigned int length);

WiFiUDP       _ntpUdp;
EasyNTPClient _ntpClient(_ntpUdp, NTP_SERVER);

time_t getNtpTime()
{
  return _ntpClient.getUnixTime();
}

class baseNode
{
public:
baseNode(storage* s) :_storage(s),_ledController(s),_mqttClient(_client)
{
  _lastWifiStatus   = WiFi.status();
  _firstTimeConnect = true;
  _mqttLastStatus   = false;
}

void setup()
{
  uint32_t now = millis();
  loadConfig();
  WiFi.hostname         (_id);

  if(_storage->getNodeConfig(WIFI_MODE) == WIFI_MASTER)
  {
    WiFi.softAP(_id.c_str(), String(WIFI_PWD).c_str());
  }
  else
  {
    WiFi.setAutoConnect   (true);
    WiFi.setAutoReconnect (true);
    //WiFi.persistent       (true);
    WiFi.mode             (WIFI_STA);
    //WiFi.setOutputPower   (0);
    //WiFi.setSleepMode     (WIFI_NONE_SLEEP);
    WiFi.begin();
  }
  yield();

  _mqttClient.setCallback(mqttCallback);
  setupComponents();

  pinMode       (_statusLedPin, OUTPUT);
  digitalWrite  (_statusLedPin, HIGH);

  _lastSensorLoopTime = now;
  _lastNodeLoopTime   = now;
  _lastLedLoopTime    = now;
  _lastOtherLoopTime  = now;
  _lastConnectAttemp  = now;

  _ntpUdp.begin (123);

  setupNode();
  Serial.println(String(F("\n\n\n ...OK! Setup took "))+String(millis())+String(F("ms")));
  delay(100);
  _storage->setKeyValue(BERROR,String(0),BOOT_CONFIG);
}

static void launchConfigPortal(storage* s)
{
  int pin = s->getNodeConfig(STATUS_PIN).toInt();
  pinMode(pin, OUTPUT);
  for(uint t = 0 ; t < 4 ; t++)
  {
    digitalWrite(pin,HIGH);
    delay(50);
    digitalWrite(pin,LOW);
    delay(50);
  }
  WiFiManager wifiManager;
  wifiManager.setTimeout(120);
  wifiManager.setBreakAfterConfig(true);

  std::vector<std::vector<WiFiManagerParameter*> > parameters;
  std::vector<WiFiManagerParameter*> titles;
  std::vector<String*>               strs;
  std::vector<String>                files;

  for (auto file : s->getFileList())
  {
    if(file.endsWith(F(".json")))
    {
      StaticJsonBuffer<JSONCONFIGSIZE>   data;
      JsonObject& cfg = data.parseObject(s->readFile(file));
      if(cfg.success())
      {
        files.push_back(file);
        parameters.push_back(std::vector<WiFiManagerParameter*>());
        strs.push_back(new String("<p><big><big>"+file+"</big></big></p>"));
        titles.push_back(new WiFiManagerParameter(strs.back()->c_str()));
        wifiManager.addParameter(titles.back());

        for(auto param : cfg)
        {
          strs.push_back(new String(F("<p></p>")));
          titles.push_back(new WiFiManagerParameter(strs.back()->c_str()));
          wifiManager.addParameter(titles.back());
          strs.push_back(new String(param.key));
          const char* name = strs.back()->c_str();
          titles.push_back(new WiFiManagerParameter(name));
          wifiManager.addParameter(titles.back());
          strs.push_back(new String(param.value.as<String>()));
          const char* dval = strs.back()->c_str();
          parameters.back().push_back(new WiFiManagerParameter(name,dval,dval,40));
          wifiManager.addParameter(parameters.back().back());
        }
      }
    }
  }

  bool portalStatus = wifiManager.startConfigPortal(String(s->getNodeConfig(NODE_ID)).c_str(),String(s->getNodeConfig(WIFI_PWD)).c_str());

  for(auto t : titles) {delete t;}
  for(uint f = 0 ; f < parameters.size() ; f++)
  {
    String file = files[f];
    for(auto p : parameters[f])
    {
      String key      = p->getID();
      String val      = p->getValue();
      String lastVal  = p->getPlaceholder();
      if(val != lastVal)
      {
        //Serial.println("Updating: "+file+"\tk: "+key+"\t lastval: "+lastVal+"\t val:"+val);
        s->setKeyValue(key,val,file);
      }
      delete p;
    }
  }
  for(auto s : strs) {delete s;}
  if (!portalStatus)
  {
    Serial.print(F("Cant connect, reset!"));
    delay(3000);
    ESP.reset();
    delay(8000);
  }
}

void loadConfig()
{
  _id               = _storage->getNodeConfig(NODE_ID);
  _nodeType         = _storage->getNodeConfig(NODE_TYPE);
  _statusLedPin     = _storage->getNodeConfig(STATUS_PIN    ).toInt();
  _sensorLoopTimer  = _storage->getNodeConfig(SENSOR_REFRESH).toInt();
  _nodeLoopTimer    = _storage->getNodeConfig(NODE_REFRESH  ).toInt();
  _ledLoopTimer     = _storage->getNodeConfig(LED_REFRESH   ).toInt();
  _otherLoopTimer   = _storage->getNodeConfig(OTHER_REFRESH ).toInt();
  _teleTimer        = _storage->getNodeConfig(TELE_REFRESH  ).toInt();
  _loopDelay        = _storage->getNodeConfig(LOOP_DELAY    ).toInt();
  _aliveTimer       = _storage->getNodeConfig(WD_REFRESH    ).toInt();
  _mqttPort         = _storage->getNodeConfig(MQTT_PORT     ).toInt();
  _mqttHost         = _storage->getNodeConfig(MQTT_SERVER);
  _mqttEnabled      = _storage->getNodeConfig(MQTT_ENABLED) == ENABLED;
  _mqttTopic        = "node/"+_id+"/";


  Serial.println(String(F("\nID: "))+_id+ String(F(" nodeType: "))+_nodeType);
  Serial.println(String(F("status_pin:"))+String(_statusLedPin)+String(F(" delay:"))+String(_loopDelay)+String(F(" alive:"))+String(_aliveTimer));
}

void update()
{
  uint32_t loopStart = millis();

  checkConnectivity();
  handleLoops();

  uint32_t sysStart = millis();

  if( (millis() - _lastAliveTime) > _aliveTimer) {imAlive();}

  if( (millis() - _lastTeleTime)  > _teleTimer)
  {
    publishDataJson();
    publishStatusJson();
    _lastTeleTime = millis();
  }

  if(_statusLedDecay > 0)
  {
    _statusLedDecay -= _lastLoopMS;
    if(_statusLedDecay <= 0)digitalWrite(_statusLedPin,HIGH);
  }
  yield();
  _sysMS += millis() - sysStart;

  uint32_t sleepTime = millis();
  if(_loopDelay > 0)
    delay(_loopDelay);
  else
  {
    uint32_t now = millis();
    //cuanto queda hasta la siguente cosa a hacer?
      int timeToUpdateSensors = _lastSensorLoopTime - now + _sensorLoopTimer;
      int timeToUpdateLeds    = _lastLedLoopTime    - now + _ledLoopTimer;
      int timeToUpdateNode    = _lastNodeLoopTime   - now + _nodeLoopTimer;
      int timeToUpdateOther   = _lastOtherLoopTime  - now + _otherLoopTimer;

      int timeToNextThing     = timeToUpdateOther;
      if( timeToUpdateLeds    < timeToNextThing) timeToNextThing = timeToUpdateLeds;
      if( timeToUpdateNode    < timeToNextThing) timeToNextThing = timeToUpdateNode;
      if( timeToUpdateSensors < timeToNextThing) timeToNextThing = timeToUpdateSensors;
      if(timeToNextThing > 0)
        delay(timeToNextThing);
  }

  uint32_t now  = millis();
  _lastLoopTime = now;
  _sleptMS     += now - sleepTime;
  _lastLoopMS   =_lastLoopTime - loopStart;
}

void handleLoops()
{
  //gestion de loops
  if(_lastLoopTime > millis())
  {//el contador ha desbordado
    _lastSensorLoopTime = 0;
    _lastNodeLoopTime   = 0;
    _lastLedLoopTime    = 0;
    _lastOtherLoopTime  = 0;
    _lastAliveTime      = 0;
    _lastConnectAttemp  = 0;
    _lastTeleTime       = 0;
  }
  _loops++;
  uint32_t now;
  now = millis();
  //Toca actualizar los sensores?
  if((now - _lastSensorLoopTime) > _sensorLoopTimer)
  {
    uint32_t startTime = millis();
    sensorLoop();
    _sensorLoops++;
    now = millis();
    _sensorLoopMS  += now - startTime;
    _lastSensorLoopTime = now;
  }
  now = millis();
  //Toca actualizar los leds?
  if((now - _lastLedLoopTime) > _ledLoopTimer)
  {
    uint32_t startTime = millis();
    ledLoop();
    _ledLoops++;
    now = millis();
    _ledLoopMS  += now - startTime;
    _lastLedLoopTime = now;
  }
  now = millis();
  //Toca actualizar el nodo?
  if((now - _lastNodeLoopTime) > _nodeLoopTimer)
  {//toca actualizar el nodo
    uint32_t startTime = millis();
    updateComponents();
    nodeLoop();
    _nodeLoops++;
    now = millis();
    _nodeLoopMS  += now - startTime;
    _lastNodeLoopTime = now;
  }
  now = millis();
  if((now - _lastOtherLoopTime) > _otherLoopTimer)
  {//toca actualizar otras cosas de menos prioridad
    uint32_t startTime = millis();
    slowLoop();
    _otherLoops++;
    now = millis();
    _otherLoopMS  += now - startTime;
    _lastOtherLoopTime = now;
  }
}

virtual void ledLoop()
{
   updateLedGadgets();
  _ledController.update();
}

virtual void slowLoop()
{
  ArduinoOTA.handle();
  publishComponents();
  if(_mqttClient.connected()) {_mqttClient.loop();}
  parseSerial();
}

void imAlive()
{
  _lastAliveTime = millis();
  _statusLedDecay = 500;
  digitalWrite(_statusLedPin,LOW);

  uint16_t loops        = _loops        / (_aliveTimer/1000.0);
  uint16_t ledLoops     = _ledLoops     / (_aliveTimer/1000.0);
  uint16_t nodeLoops    = _nodeLoops    / (_aliveTimer/1000.0);
  uint16_t sensorLoops  = _sensorLoops  / (_aliveTimer/1000.0);
  uint16_t otherLoops   = _otherLoops   / (_aliveTimer/1000.0);
  _loops = 0, _ledLoops = 0, _nodeLoops = 0, _sensorLoops = 0, _otherLoops = 0;

  uint16_t sysTime       = _sysMS        / (_aliveTimer/1000.0);
  uint16_t sleptTime     = _sleptMS      / (_aliveTimer/1000.0);
  uint16_t ledTime       = _ledLoopMS    / (_aliveTimer/1000.0);
  uint16_t sensorTime    = _sensorLoopMS / (_aliveTimer/1000.0);
  uint16_t nodeTime      = _nodeLoopMS   / (_aliveTimer/1000.0);
  uint16_t otherTime     = _otherLoopMS  / (_aliveTimer/1000.0);
  _sysMS = 0, _sleptMS = 0,_ledLoopMS = 0,_sensorLoopMS = 0,_nodeLoopMS = 0,_otherLoopMS = 0;
  _lastSys = sysTime;_lastSlept = sleptTime;
  float mqttPackets   = _mqttPackets  / (_aliveTimer/1000.0);
  _mqttPackets = 0;

  float artnetPackets = _ledController.getArtnetrx() / (_aliveTimer/1000.0);
  float sacnPackets   = _ledController.getsACNrx()   / (_aliveTimer/1000.0);
  float fps           = _ledController.getFrames()   / (_aliveTimer/1000.0);

  Serial.println(String(F("\n*Im alive!"))+F("\tUptime: ")+String(millis()/1000.0)+"s\tTime:"+String(now()));
  Serial.println(String(F("|-Node ID:")) + _id + String(F("\tType: ")) + _nodeType + String(F("\tESSID: "))+String(WiFi.SSID())+String(F("\tStatus: ")) + String(WiFi.status()) + String(F("\tSignal:")) + String(WiFi.RSSI()) + String(F("dbm\tIP: ")) + localIP());
  Serial.println(String(F("|-Leds:")) + String(_ledController.ledCount()) + String(F("\tMaxPower:")) +String(_ledController.maxPower())+ String(F("\tBright::")) +String(_ledController.brightness())+ String(F("\tUnderVoltDimmer:")) +String(_ledController.undervoltDimmer()));
  Serial.println(String(F("|")));
  Serial.println(String(F("|-Led Frames/s: ")) + String(fps)+F("\tVCC:") + String(ESP.getVcc()/1000.0) + String(F("\tFreemem:")) +formatBytes(system_get_free_heap_size()));
  Serial.println(String(F("|")));
  Serial.println(String(F("|-Loop delay:\t")) +String(_loopDelay)  +String(F("\tnode:\t")) +String(_nodeLoopTimer) +String(F("\tled:\t"))   +String(_ledLoopTimer) +String(F("\tsensor:\t")) +String(_sensorLoopTimer) +String(F("\tother:\t")) +String(_otherLoopTimer));
  Serial.println(String(F("|-Loops/s:\t"))    +String(loops)       +String(F("\tl/s:\t"))  +String(nodeLoops)      +String(F("\tl/s:\t"))   +String(ledLoops)      +String(F("\tl/s:\t"))    +String(sensorLoops)      +String(F("\tl/s:\t"))   +String(otherLoops));
  Serial.println(String(F("|-LastTime:\t"))   +String(_lastLoopMS) +String(F("ms \tnT\t")) +String(nodeTime)       +String(F("ms \tlT:\t")) +String(ledTime)       +String(F("ms \tsT:\t"))  +String(sensorTime)       +String(F("ms \tot:\t")) +String(otherTime) +String(F("ms\tSysTime:"))+String(sysTime) +String(F("ms \tslept: "))+String(sleptTime)  +String(F("ms")));
  Serial.println(String(F("|")));
  Serial.println(String(F("|-RCV: mqtt/s: "))+String(mqttPackets)+String(F("\tartnet/s: "))+String(artnetPackets)+String(F("\tsacn/s: "))+String(sacnPackets));
  Serial.println(String(F("|___")));
}


void mqttRX()
{
  _statusLedDecay = 50;
  digitalWrite(_statusLedPin,LOW);
  _mqttPackets++;
}

ledController* lController()
{
  return &_ledController;
}

void readTopic(char* topic, byte* payload, unsigned int length)
{
  mqttRX();
  for(uint c = 0 ; c < _components.size() ; c++)
    if(_components[c]->readTopic(topic,payload,length))
        return;
  readTopicNode(topic,payload,length);
}

void subscribeMQTT(String topic)
{
  Serial.println(String(F("Subscribe :"))+topic);
  _mqttClient.subscribe(topic.c_str());
//  _mqttClient.loop();
}

virtual void subscribeTopics()
{
  subscribeMQTT(String(_mqttTopic+String(F("cmnd"))).c_str());
  for(auto t : _subscriptions)
    subscribeMQTT(t);
  for(uint c = 0 ; c < _components.size() ; c++)
  {
    std::vector<String> topics = _components[c]->getTopics();
    for( auto t : topics)
      subscribeMQTT(String(_mqttTopic+t).c_str());
  }
}

void publishMQTT(String topic,String& payload,bool persist = false)
{
  if(!_mqttClient.publish(topic.c_str(),(uint8_t*)payload.c_str(),payload.length(),persist))
  {
    Serial.println(String(F("publishError!! - "))+topic+ String(F(" - size:"))+String(payload.length()));
  }
}

void publishMQTT(mqttPub& pub)
{
  publishMQTT(pub.topic,pub.payload,pub.persist);
}

virtual void setupNode()  {;}
virtual void sensorLoop() {;}
virtual void nodeLoop()   {;}
virtual bool readTopicNode(char* topic, byte* payload, unsigned int length){return false;}

protected:
  String        _id;
  String        _nodeType;
  int           _statusLedPin;

  storage*                    _storage;
  ledController               _ledController;
  std::vector<nodeComponent*> _components;
  std::vector<nodeComponent*> _ledGadgets;
  std::vector<String>         _subscriptions;

  WiFiClient    _client;
	PubSubClient  _mqttClient;
  bool          _mqttEnabled;
  bool          _mqttLastStatus;
  String        _mqttHost;
  uint16_t      _mqttPort;
  String        _mqttTopic;
  uint32_t      _lastConnectAttemp;

  uint8_t       _lastWifiStatus;
  uint8_t       _firstTimeConnect;
//temporizadores para stadisticas  y gestion de loops
  uint16_t _loops             = 0;
  uint16_t _sensorLoops       = 0;
  uint16_t _nodeLoops         = 0;
  uint16_t _ledLoops          = 0;
  uint16_t _otherLoops        = 0;

  uint16_t _sensorLoopTimer   = 10;
  uint16_t _nodeLoopTimer     = 100;
  uint16_t _ledLoopTimer      = 10;
  uint16_t _otherLoopTimer    = 250;
  uint16_t _aliveTimer        = 5000;
  uint32_t _teleTimer         = 60000;

  uint32_t _lastLoopTime       = 0;
  uint32_t _lastSensorLoopTime = 0;
  uint32_t _lastNodeLoopTime   = 0;
  uint32_t _lastLedLoopTime    = 0;
  uint32_t _lastOtherLoopTime  = 0;
  uint32_t _lastAliveTime      = 0;
  uint32_t _lastTeleTime       = 0;


  uint16_t _lastLoopMS     = 0;
  uint16_t _sensorLoopMS   = 0;
  uint16_t _ledLoopMS      = 0;
  uint16_t _nodeLoopMS     = 0;
  uint16_t _otherLoopMS    = 0;
  uint16_t _sysMS          = 0;
  uint16_t _sleptMS        = 0;
  uint16_t _lastSys        = 0;
  uint16_t _lastSlept      = 0;

  uint16_t _loopDelay      = 0;

  int16_t _statusLedDecay  = 0;

  uint16_t _mqttPackets    = 0;

  void parseSerial()
  {
    if(Serial.available())
    {
      char cx = Serial.read();
      if( cx == 'C')
      {
        launchConfigPortal(_storage);
      }
    }
  }

  void checkConnectivity()
  {
    uint8_t s = WiFi.status();
    if(WiFi.status() != _lastWifiStatus)
    {
      _lastWifiStatus = s;
      if(s == 3) wifiConnected();
      else       wifiDisconnected();
    }
    else if(s == 3)
    {
      if(_mqttEnabled == true) checkMQTT();
    }
  }

  void checkMQTT()
  {
    if(!_mqttClient.connected())
    {
      if(_mqttLastStatus)
      {
        _mqttLastStatus = false;
        mqttDisconnected();
      }
      uint32_t now = millis();
      if((now - _lastConnectAttemp) > 30000)
      {
        _lastConnectAttemp = now;
        connectMQTT();
      }
    }
  }

  void connectMQTT()
  {
    _mqttClient.setServer(_mqttHost.c_str(),_mqttPort);
    Serial.println(String(F("MQTT connecting:"))+_mqttHost+":"+String(_mqttPort));
    if(_mqttClient.connect(_id.c_str()))
      initMQTT();
    else
      Serial.println(F("Can't connect! :("));
  }

  void initMQTT()
  {
    _mqttLastStatus = true;
    delay(100);
    mqttConected();
    _mqttClient.publish("node/connected", _id.c_str(), false);
    _mqttClient.loop();
    subscribeTopics();
  }


  virtual void mqttConected()
  {
    Serial.println(String(F("MQTT Connected!")));
  }

  virtual void mqttDisconnected()
  {
    Serial.println(String(F("MQTT Disconnected!")));
  }

  virtual void wifiDisconnected()
  {
    Serial.println(String(F("Wifi Disconnected!")));
  }

  virtual void wifiConnected()
  {
    Serial.println(String(F("Wifi Connected!")));
    if(_firstTimeConnect)
      firstTimeConnect();
  }

  void firstTimeConnect()
  {
      delay(1000);
      _firstTimeConnect = false;
      _ledController.initDMX();
      if(MDNS.begin(_id.c_str()))
      {
          Serial.println(String(F("MDNS started, name:"))+_id);
          MDNS.addService("http", "tcp", 80);
      }
      Serial.print(String(F("Sync NTP Time, current:"))+String(now())+"...");
      setSyncProvider(getNtpTime);
      setSyncInterval(NTP_SYNCTIME);
      Serial.println(String(F("Sync:"))+String(now()));
      if(_mqttEnabled) {connectMQTT();}
  }

  void addComponent(nodeComponent* c) {_components.push_back(c);}
  void setupComponents()              {for(uint c = 0; c < _components.size() ; c++) _components[c]->setup();}
  void updateComponents()             {for(uint c = 0; c < _components.size() ; c++) _components[c]->update();}

  void addledGadget    (ledGadget* c) {_ledGadgets.push_back(c);}
  void updateLedGadgets()             {for(uint c = 0; c < _ledGadgets.size() ; c++) _components[c]->update();}

  void publishDataJson()
  {
    StaticJsonBuffer<JSONCONFIGSIZE> data;
    JsonObject& json = data.createObject();
    json["id"]       = _id;
    json["time"]     = String(now());
    JsonObject& d    = json.createNestedObject("data");
    for(uint c = 0 ; c < _components.size() ; c++)
      _components[c]->getJsonData(d);
    String topic = _mqttTopic+"data";
    String p;
    json.printTo(p);
    //Serial.println(topic+"\n"+p);
    publishMQTT(topic,p);
  }

  void publishStatusJson()
  {
    StaticJsonBuffer<JSONCONFIGSIZE> data;
    JsonObject& json  = data.createObject();
    json["id"]        = _id;
    json["chipId"]    = String(ESP.getChipId(), HEX);
    json["time"]      = String(now());
    json["uptime"]    = String(millis()/1000.0);
    json["vcc"]       = ESP.getVcc()/1000.0;
    json["freeMem"]   = formatBytes(system_get_free_heap_size());
    json["sysTime"]   = _lastSys;
    json["sleepTime"] = _lastSlept;
    JsonObject& w     = json.createNestedObject("wifi");
    w["signal"]       = String(WiFi.RSSI());

    String topic = _mqttTopic+"status";
    String p;
    json.printTo(p);
    //Serial.println(topic+"\n"+p);
    publishMQTT(topic,p);
  }

  void publishComponents()
  {
    for(uint c = 0 ; c < _components.size() ; c++)
    {//ojo al orden de las publicaciones cuando se encolan
      std::vector<mqttPub> pubs = _components[c]->getPublications();
      while (pubs.size())
      {
        mqttPub p = pubs[0];
        pubs.erase(pubs.begin());
        p.topic = _mqttTopic+p.topic;
        publishMQTT(p);
      }
    }
  }

};

#endif
