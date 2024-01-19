#ifndef NODECOMPONENT
#define NODECOMPONENT

#include "storage.h"

class nodeComponent
{
public:
  nodeComponent(String id,storage* s,String unit ="") : _id(id), _storage(s),_unit(unit)
  {

  }

  virtual void setup()
  {
    Serial.println("Component:"+ _id);

    checkConfig();
    setupComponent();
  }

  virtual void update()
  {
    updateComponent();
    if(_pollInterval > 0)
    {
      uint32_t now = millis();
      if(_lastPollTime > now)
        _lastPollTime = 0;
      if(now - _lastPollTime > _pollInterval)
      {
        _lastPollTime = now;
        pollData();
      }
    }
  }

  void pollData()
  {
    StaticJsonBuffer<JSONCONFIGSIZE> data;
    JsonObject& json = data.createObject();
    getJsonData(json);
    if(json.size() > 0)
    {
      String payload;
      JsonObject& d = json[_id];
      if(d.size())
      {
        d.printTo(payload);
        publish(F("data"),payload);
        for (auto v : d)
          publish(v.key,v.value);
      }
    }
  }

  void setUnit(String u) {_unit = u;}

  std::vector<mqttPub> getPublications()
  {
    std::vector<mqttPub> result;
    while (_pubBuffer.size()) {
      result.push_back(_pubBuffer[0]);
      _pubBuffer.erase(_pubBuffer.begin());
    }
    return result;
  }

  std::vector<String> getTopics() {return _subscribeTopics;}
  void addTopic(String topic){_subscribeTopics.push_back(String(_id+"/"+topic));}

//OVERLOAD!
  virtual bool    readTopic(char* topic, byte* payload, unsigned int length) {return false;}
  virtual void    updateComponent()             {}
  virtual void    setupComponent ()             {}
  virtual void    sensorLoop     ()             {}
  virtual void    getJsonData(JsonObject& data) {}
//

protected:
  String   _id;
  storage* _storage;
  String   _unit;

  uint32_t _pollInterval   = 60000;
  uint32_t _lastPollTime   = 0;

  std::vector<mqttPub> _pubBuffer;
  std::vector<String>  _subscribeTopics;


  virtual void publish(String topic,String data,bool persist = false)
  {
    topic = _id+"/"+topic;
    _pubBuffer.push_back(mqttPub(topic,data,persist));
  }

  virtual void publish(mqttPub& pub)
  {
    pub.topic = _id+"/"+pub.topic;
    _pubBuffer.push_back(pub);
  }

  virtual void checkConfig()
  {
    yield();
    StaticJsonBuffer<JSONCONFIGSIZE> data;
    JsonObject& cfg = data.parseObject(_storage->readFile("/"+_id+".json"));
    if (cfg.success() && (cfg.size()>0))
    {
      yield();
      StaticJsonBuffer<JSONCONFIGSIZE> dataS;
      JsonObject& scfg = dataS.createObject();
      initConfig(scfg);
      if(cfg["v"] || scfg["v"])
      {
        int sv = scfg["v"].as<int>();
        int v =   cfg["v"].as<int>();
        if(!cfg["v"]) v = 0;
        if(sv > v)
        {//actualizar config
          for(auto val : cfg)
            scfg[val.key] = val.value;
          for(auto val : scfg)
            cfg[val.key] = val.value;
          cfg["v"] = String(sv);
          String json;
          cfg.printTo(json);
          _storage->writeFile("/"+_id+".json",json);
        }
      }
      cfg.prettyPrintTo(Serial);
      loadConfig(cfg);
    }
    else
    {
      StaticJsonBuffer<JSONCONFIGSIZE> data2;
      JsonObject& cfg2 = data.createObject();
      initConfig(cfg2);
      if(cfg2.size() > 0)
      {
        String json;
        cfg2.printTo(json);
        _storage->writeFile("/"+_id+".json",json);
        yield();
        cfg2.prettyPrintTo(Serial);
        loadConfig(cfg2);
      }
    }
  }

  virtual void initConfig(JsonObject& cfg) {;}
  virtual void loadConfig(JsonObject& cfg) {;}
};


class pushButton : public nodeComponent
{
public:
  pushButton(String id,storage* s) : nodeComponent(id,s,F("Enabled")) { }

  virtual void initConfig(JsonObject& cfg)
  {
    cfg["pin"] = D2;
  }

  virtual void loadConfig(JsonObject& cfg)
  {
    _pin = cfg["pin"];
  }

  virtual void getJsonData(JsonObject& data)
  {
    JsonObject& d = data.createNestedObject(_id);
    d["status"] = String(read());
  }

  virtual void setupComponent()
  {
    pinMode(_pin,INPUT);
    _lowState  = digitalRead(_pin);
    _lastState = false;
  }

  virtual void updateComponent()
  {
    bool status = read();
    if(status != _lastState)
      if(status)
      {
        _longPressed      = false;
        _pressed          = true;
        _startPressTime   = millis();
        _partialPressTime = _startPressTime;
        publish("status", "1");
        publish("press" , "1");
      }
      else
      {
        _pressTime = millis() - _startPressTime;
        publish("status"        , "0");
        publish("release"       , String(_pressTime));
      }
    else if(status)
    {//El boton se mantiene presionado
      if((millis()-_startPressTime > 2500) && !_longPressed)
      {
          _longPressed = true;
          publish("longPress" , "1");
      }
      if((millis()-_partialPressTime > 500))
      {
        _partialPressTime = millis();
        publish("pressed" , String(millis()-_startPressTime));
      }
    }

    _lastState = status;
  }

  bool pressed()
  {
    bool r = _pressed;
    _pressed = false;
    return r;
  }

  bool longPressed()
  {
    bool r = _longPressed;
    _longPressed = false;
    return r;
  }

  uint16_t released()
  {
    uint16_t r = _pressTime;
    _pressTime = 0;
    return r;
  }

  bool status() { return _lastState; }
  bool read()   {return digitalRead(_pin) != _lowState;}


protected:
  int       _pin;
  bool      _lastState;
  bool      _lowState;
  bool      _pressed;
  bool      _longPressed;
  uint32_t  _startPressTime;
  uint32_t  _partialPressTime;
  uint16_t  _pressTime;

};

#endif
