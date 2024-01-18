#ifndef STORAGESS
#define STORAGESS

#include "extra.h"
#include "strings.h"

//

struct mqttPub
{
  mqttPub(String& t,String& p,bool ps = false) : topic(t),payload(p),persist(ps) {}
  mqttPub() : topic(),payload(),persist(false) {}
  String topic;
  String payload;
  bool   persist;
};

class storage
{
public:
  storage()
  {
  }

String stockNodeConfig()
{
  StaticJsonBuffer<JSONCONFIGSIZE> data;
  JsonObject& root = data.createObject();

  //root[NODE_ID]             = String(F("Node-"))+String(ESP.getChipId(), HEX);
  //root[NODE_TYPE]           = F("new");//new;
  root[NODE_ID]             = "ledTree";
  root[NODE_TYPE]           = F("ledTree");//new;
  root[FIRST_RUN]           = F("Y");

  root[WIFI_MODE]           = WIFI_CLIENT;
  root[WIFI_ESSID]          = root[NODE_ID];
  root[WIFI_PWD]            = "configureme";

  root[STATIC_IP_EN]        = DISABLED;
  root[STATIC_IP]           = F("10.0.1.4");

  root[MQTT_ENABLED]        = ENABLED;
  root[MQTT_SERVER]         = F("192.168.10.10");
  root[MQTT_PORT]           = F("1883");

  root[STATUS_PIN]          = LED_BUILTIN;
  //root[STATUS_PIN]          = D4;
  root[CONFIG_PIN]          = D1;
  root[CONFIG_MODE]         = ON_BOOT_ERROR;
  root[MAX_BERROR]          = F("5");
  root[BERROR]              = F("0");

  root[LOOP_DELAY]          = F("0");
  root[SENSOR_REFRESH]      = F("1");
  root[LED_REFRESH]         = F("5");
  root[NODE_REFRESH]        = F("50");
  root[OTHER_REFRESH]       = F("100");
  root[WD_REFRESH]          = F("5000");
  root[TELE_REFRESH]        = F("60000");

  root[NODE_CONFIG_VERSION] = F("2");

  String result;
  root.printTo(result);
  return result;
}

String stockLedConfig()
{  ///Pasar toda esta mierda a defines para ahorrar memo
  StaticJsonBuffer<JSONCONFIGSIZE> data;
  JsonObject& root = data.createObject();

  root[ARTNET_ENABLED]      = ENABLED;
  root[ARTNET_UNIVERSE]     = F("1");
  root[ARTNET_CHANNEL]      = F("1");
  root[ARTNET_ANNOUNCE]     = ENABLED;

  root[SACN_ENABLED]        = ENABLED;
  root[SACN_UNIVERSE]       = F("1");
  root[SACN_CHANNEL]        = F("1");

  root[LED_HW]              = F("ws2812");
  root[LED_COUNT]           = F("80");
  root[LED_BRIGHT]          = F("1");
  root[LED_MAXBRIGHT]       = F("230");
  root[UNDERVOLT_PROTECT]   = ENABLED;
  root[POWER_CALIBRATION]   = ENABLED;

  root[LEDS_CONFIG_VERSION] = F("1");

  String result;
  root.printTo(result);
  return result;
}

virtual String  getNodeConfig(String key)                 {return getKeyValue(key,NODE_CONFIG);}
virtual String  getLedConfig (String key)                 {return getKeyValue(key,LEDS_CONFIG);}
virtual bool    setNodeConfig(String key,String val)      {return setKeyValue(key,val,NODE_CONFIG);}
virtual bool    setLedConfig (String key,String val)      {return setKeyValue(key,val,LEDS_CONFIG);}

virtual void    initConfig()                        {;}
virtual void    dumpConfig()                        {;}
virtual String  readFile(String file)               {return "";}
virtual void    writeFile(String file,String& data) {;}
virtual bool    hasStoredConfig()                   {return false;}
virtual String  getKeyValue(String key, String filename)               = 0;
virtual bool    setKeyValue(String key,String value, String filename)  = 0;

virtual std::vector<String> getFileList() {return std::vector<String>();}

protected:




};


class spifsStorage : public storage
{
public:
  spifsStorage() : storage()
  {
    Serial.print(F("Opening SPIFFS..."));
    if(SPIFFS.begin())
    {
      Serial.println(F("OK!"));
      Dir dir = SPIFFS.openDir("/");
      while (dir.next()) {
        String fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
        Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
      }
      Serial.printf("\n");
    }
    else
    {
        Serial.println(F("\n SPIFFS FAIL!"));
    }

    if(!hasNodeConfig())
    {
      initNodeConfig();
    }
    if(!hasLedConfig())
    {
      initLedConfig();
    }

    dumpConfig();
  }

  virtual String getKeyValue(String key, String filename)
  {
    StaticJsonBuffer<JSONCONFIGSIZE>  data;
		File f = SPIFFS.open(filename, "r");
		if (f) {
			size_t size = f.size();
			if ( size == 0 ) {
				f.close();
				return "";
			}
      else
      {
				std::unique_ptr<char[]> buf (new char[size]);
				f.readBytes(buf.get(), size);
				JsonObject& cfg = data.parseObject(buf.get());
				if (!cfg.success()) {
					return "";
				}
			f.close();
			return cfg[key];
			}
		}
    else
		{
			return "";
		}
  }

  virtual bool setKeyValue(String key,String value, String filename)
  {
		File f = SPIFFS.open(filename, "r");
		if ( (f) && (f.size() >0) )
		{
      StaticJsonBuffer<JSONCONFIGSIZE>  data;
			size_t size = f.size();
			std::unique_ptr<char[]> buf (new char[size]);
			f.readBytes(buf.get(), size);
			JsonObject& cfg = data.parseObject(buf.get());
			if (cfg.success()) {
				cfg[key] = value;
				f.close();
				f = SPIFFS.open(filename, "w");
				cfg.printTo(f);
				f.close();
				return true;
			}
			else
			{
				Serial.println(F("Bad JSON :("));
				return false;
			}

		}
		 else
		{
			StaticJsonBuffer<JSONCONFIGSIZE> data;
			JsonObject& root = data.createObject();
			root[key] = value;
			f = SPIFFS.open(filename, "w");
			root.printTo(f);
			f.close();
			return true;
		}
  }

  virtual String readFile(String file)
  {
    String result;
  	File f = SPIFFS.open(file, "r");
  	if (f) {
  		size_t size = f.size();
  		if ( size == 0 ) {
  			f.close();
  			Serial.println(F("Bad File!"));
  			return "";
  		}else{
        while(f.available())
        {
          result += (char)f.read();
        }
        f.close();
        return result;
  		}
  	}
    else
  	{
  		Serial.println(String(F("cannot open:")) + file);
  		return "";
  	}
  }

  virtual void writeFile(String file,String& data)
  {
    File f;
    f = SPIFFS.open(file, "w");
    if (f)
    {
      for(uint i = 0 ; i < data.length(); i++)
      {
        f.write(data.charAt(i));
      }
      f.close();
    }
    else
    {
      Serial.print(String(F("cannot write to "))+file);
    }
  }

  virtual void initNodeConfig()
  {
    Serial.println(String(F("Writing node config!")));
    StaticJsonBuffer<JSONCONFIGSIZE>  data;
    JsonObject& ncfg = data.parseObject(stockNodeConfig());
    if (!ncfg.success())
    {
      Serial.println(String(F("ERROR")));
      return;
    }
    Serial.print(String(F("Node config. size: "))+String(stockNodeConfig().length())+" ,");
    Serial.print(ncfg.size());
    Serial.println(String(F(" records found.")));
    yield();
    File f;
    f = SPIFFS.open(NODE_CONFIG, "w");
    if(f)
    {
      ncfg.printTo(f);
      f.close();
    }
    else
    {
      Serial.println(F("cannot open file"));
    }
  }

  virtual void initLedConfig()
  {
    Serial.println(String(F("Writing led config!")));
    yield();
    StaticJsonBuffer<JSONCONFIGSIZE>  data;
    JsonObject& lcfg = data.parseObject(stockLedConfig());

    if (!lcfg.success())
    {
      Serial.println(String(F("ERROR!")));
      yield();
      return;
    }
    Serial.print(String(F("Led config. size: "))+String(stockLedConfig().length())+" ,");
    Serial.print(lcfg.size());
    Serial.println(String(F(" records found.")));
    yield();
    File f;
    f = SPIFFS.open(LEDS_CONFIG, "w");
    if(f)
    {
      lcfg.printTo(f);
      f.close();
    }
    else
    {
      Serial.println(F("cannot open file"));
    }
  }


  virtual bool hasNodeConfig()
  {
    File f = SPIFFS.open(NODE_CONFIG, "r");
		if ( (f) && (f.size() >0) )
		{
      StaticJsonBuffer<JSONCONFIGSIZE>  data;
			size_t size = f.size();
			std::unique_ptr<char[]> buf (new char[size]);
			f.readBytes(buf.get(), size);
      f.close();
			JsonObject& cfg = data.parseObject(buf.get());

      if (!cfg.success()) {
    		Serial.println(F("Parsing json failed !"));
    		return false;
    	}
      String version = cfg[NODE_CONFIG_VERSION];
      if(version.toInt() > 0)
      {
        Serial.println(String(F("Found node config v:"))+version);
        StaticJsonBuffer<JSONCONFIGSIZE>  data2;
      	JsonObject& scfg = data2.parseObject(stockNodeConfig());
        if (!scfg.success()) {
          Serial.println(F("ERROR!"));
        }
        else
        {
          String sver = scfg[NODE_CONFIG_VERSION];
          Serial.println(String(F("Stock version: "))+sver);
          if(sver.toInt() > version.toInt())
          {
            String(F(" updating..."));
            for (auto kvp : cfg) {
              scfg[kvp.key] = kvp.value;
            }
            scfg[NODE_CONFIG_VERSION] = sver;
            f = SPIFFS.open(NODE_CONFIG, "w");
            if(f)
            {
              scfg.printTo(f);
              f.close();
            }
          }
        }
        return true;
      }
      else
        return false;
    }
    else
      return false;
  }

  virtual bool hasLedConfig()
  {
    File f = SPIFFS.open(LEDS_CONFIG, "r");
		if ( (f) && (f.size() >0) )
		{
      StaticJsonBuffer<JSONCONFIGSIZE>  data;
			size_t size = f.size();
			std::unique_ptr<char[]> buf (new char[size]);
			f.readBytes(buf.get(), size);
      f.close();
			JsonObject& cfg = data.parseObject(buf.get());

      if (!cfg.success()) {
    		Serial.println(F("Parsing json failed !"));
    		return false;
    	}
      String version = cfg[LEDS_CONFIG_VERSION];
      if(version.toInt() > 0)
      {
        Serial.println(String(F("Found leds config v:"))+version);
        StaticJsonBuffer<JSONCONFIGSIZE>  data2;
      	JsonObject& scfg = data2.parseObject(stockLedConfig());
        if (!scfg.success()) {
          Serial.println(F("ERROR!"));
        }
        else
        {
          String sver = scfg[LEDS_CONFIG_VERSION];
          Serial.println(String(F("Stock version: "))+sver);
          if(sver.toInt() > version.toInt())
          {
            String(F(" updating..."));
            for (auto kvp : cfg) {
              scfg[kvp.key] = kvp.value;
            }
            scfg[NODE_CONFIG_VERSION] = sver;
            f = SPIFFS.open(LEDS_CONFIG, "w");
            if(f)
            {
              scfg.printTo(f);
              f.close();
            }
          }
        }
        return true;
      }
      else
        return false;
    }
    else
      return false;
  }

  void dumpConfig()
  {
    StaticJsonBuffer<JSONCONFIGSIZE>  data;
  	JsonObject& cfg = data.parseObject(readFile(NODE_CONFIG));
  	if (!cfg.success()) {
  		return;
  	}
    cfg.prettyPrintTo(Serial);
    Serial.println("");
    JsonObject& cfg2 = data.parseObject(readFile(LEDS_CONFIG));
  	if (!cfg2.success()) {
  		return;
  	}
    cfg2.prettyPrintTo(Serial);
    Serial.println("");
  }

  virtual std::vector<String> getFileList()
  {
    std::vector<String> r;
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      r.push_back(fileName);
    }
    return r;
  }


protected:

};

#endif //STORAGESS
