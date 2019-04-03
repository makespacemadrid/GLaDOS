#ifndef SPIFS_H
#define SPIFS_H

#define JSONCONFIGSIZE  500
#define JSONAUTHSIZE    1000

WiFiUDP udp;
EasyNTPClient ntpClient(udp, "es.pool.ntp.org");

struct msmUserAccess
{
  String area     = "None";
  String cardid   = "0000000000000000";
  String nickname = "nobody";
  int    level    = -1;
};

class settingsStorage
{
public:
  settingsStorage()
  {

  }

  virtual void setup()
  {

  }

  virtual String readConfig(String field)
  {

  }

  virtual bool writeConfig(String field, String val)
  {

  }

};

#ifdef ESP32

class ESP32Storage: public settingsStorage
{
public:
  ESP32Storage() : settingsStorage()
  {

  }

  virtual void setup()
  {

  }

  virtual String readConfig(String field)
  {

  }

  virtual bool writeConfig(String field, String val)
  {

  }
};

#else

class SPIFSStorage : public settingsStorage
{
public:
  SPIFSStorage() : settingsStorage()
  {
    setup();
  }

  void initSettings()
  {
    StaticJsonBuffer<JSONCONFIGSIZE> data;
    JsonObject& root = data.createObject();
    File f;
    root["nodeID"]            = "unamedNode";
    root["nodeType"]          = "unconfiguredNode";
    root["wifiESSID"]         = "GlaDOS";
    root["wifiPWD"]           = "clubmate";
    root["overmind"]          = "192.168.10.10";
    root["overmindPort"]      = "1883";
    root["authNode"]          = "false";
    root["authServer"]        = "192.168.10.10:8000";
    root["authArea"]          = "none";
    root["updateServer"]      = "true";
    root["updateServerPort"]  = "8080";
    root["fileServer"]        = "true";
    root["fileServerPort"]    = "8000";
    root["otaupdate"]         = "true";
    root["otaserver"]         = "192.168.10.10:8000";
    root["configMode"]        = "onReset";
    root["maxFailBootConfig"] = "10";
    root["brainless"]         = "true";
    root["globalShutdown"]    = "false";
    root["configVersion"]     = "1";


    f = SPIFFS.open("/config.json", "w");
    root.printTo(f);
    f.close();
    yield();
  }

  void setup()
  {
      Serial.print("Loading SPIFFS...");
      if(SPIFFS.begin())
      {
        Serial.println("OK!");
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
          String fileName = dir.fileName();
          size_t fileSize = dir.fileSize();
          Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
        }
        Serial.printf("\n");
        if(!hasConfig())
          initSettings();
      }
      else
      {
          Serial.println("spiffs FAIL!");
      }
      udp.begin(123);
  }

  String formatBytes(size_t bytes){
    if (bytes < 1024){
      return String(bytes)+"B";
    } else if(bytes < (1024 * 1024)){
      return String(bytes/1024.0)+"KB";
    } else if(bytes < (1024 * 1024 * 1024)){
      return String(bytes/1024.0/1024.0)+"MB";
    } else {
      return String(bytes/1024.0/1024.0/1024.0)+"GB";
    }
  }


	bool hasConfig()
	{
		StaticJsonBuffer<JSONCONFIGSIZE>  data;
		File f = SPIFFS.open("/config.json", "r");
		if (f) {
			size_t size = f.size();
			if ( size == 0 ) {
				f.close();
				Serial.println("Bad config File!");
				return false;
			}else{
				std::unique_ptr<char[]> buf (new char[size]);
				f.readBytes(buf.get(), size);
				JsonObject& cfg = data.parseObject(buf.get());
				if (!cfg.success()) {
					Serial.println("Parsing json failed !");
					return false;
				}
  			f.close();
  			Serial.print("Config from SPIFFS loaded: ");
  			Serial.print(cfg.size());
  			Serial.println(" records found.");
        dumpConfig();
        String version = cfg["configVersion"];
        if( version != "")
        {
            Serial.println("Found settings, version :"+version);
        }
        else
        {
          Serial.println("Found json but np configVersion field");
          return false;
        }
			}
		} else
		{
			Serial.println("No Config File found!");
			return false;
		}
	}

  bool isAuthNode()
  {
      return readConfig("authNode") == "true";
  }

  bool updateUserAuth()
  {
    bool updated = false;
    updated = updated || updateAuthJSON("sideDoor");
    updated = updated || updateAuthJSON("laser");
    return updated;
  }

  bool updateAuthJSON(String name)
  {
    Serial.print("Updating "+name);
    StaticJsonBuffer<JSONAUTHSIZE>  data;
    int localVersion = 0;
		File f = SPIFFS.open("/"+name+".json", "r");
		if (f) {
			size_t size = f.size();
			if ( size == 0 ) {
				f.close();
				Serial.println("Bad config File!");
			}else{
				std::unique_ptr<char[]> buf (new char[size]);
				f.readBytes(buf.get(), size);
				JsonObject& cfg = data.parseObject(buf.get());
				if (!cfg.success()) {
					Serial.println("Parsing json failed !");
				}
  			f.close();
  			Serial.print(name+" from SPIFFS loaded: ");
  			Serial.print(cfg.size());
  			Serial.println(" records found.");
        String version = cfg["version"];
        if(version != "")
  			   localVersion = version.toInt();
        else
          Serial.println("Found json but no version field");
			}
		} else
		{
			Serial.println("No"+name+" File found!");
		}
    Serial.print("local auth version: ");Serial.println(localVersion);

    //String url = readConfig("authJSONUrl");
    String url = readConfig("authServer")+"/"+name+".json";
    Serial.println("Fetching last "+name+": " +url);
    HTTPClient http;
    http.begin(url);
    if(http.GET() == HTTP_CODE_OK) {
      String payload = http.getString();
      StaticJsonBuffer<JSONAUTHSIZE>  data;
      Serial.print("DATA; ");Serial.println(payload);
      JsonObject& cfg = data.parseObject(payload);
      if (cfg.success()) {
				int remoteVersion = 0;
        String version = cfg["version"];
        remoteVersion = version.toInt();
        Serial.print("Remote version: ");Serial.println(version);
        if(remoteVersion > localVersion)
        {
          Serial.println("updating....");
          f = SPIFFS.open("/"+name+".json", "w");
          cfg.printTo(f);
          cfg.prettyPrintTo(Serial);
          f.close();
          yield();
          return true;
        }
      }
      else
      {
        Serial.println("BAD JSON");
      }
    }
    else
    {
      Serial.println("Cannot get remote "+name);
    }
    return false;
  }

  msmUserAccess userAuthLevel(String area, String cardid)
  {
    msmUserAccess msmUser;
    msmUser.cardid = cardid;

    StaticJsonBuffer<JSONAUTHSIZE>  data;
		File f = SPIFFS.open("/"+area+".json", "r");
		if (f) {
			size_t size = f.size();
			if ( size == 0 ) {
				f.close();
				return msmUser;
			}else{
				std::unique_ptr<char[]> buf (new char[size]);
				f.readBytes(buf.get(), size);
				JsonObject& cfg = data.parseObject(buf.get());
				if (!cfg.success()) {
					return msmUser;
				}
			  f.close();
        JsonObject& user = cfg[cardid];
        if (!user.success()) {
          Serial.print("User not found");
          return msmUser;
        }
        const char* u = user["u"];
        const char* lev = user["l"];
        msmUser.nickname = u;
        msmUser.level    = String(lev).toInt();
			  return msmUser;
			}
		} else
		{
			return msmUser;
		}
  }


	void dumpConfig()
	{
		StaticJsonBuffer<JSONCONFIGSIZE>  data;
		File f = SPIFFS.open("/config.json", "r");
		if ( (f) && (f.size() >0) )
		{
			size_t size = f.size();
			std::unique_ptr<char[]> buf (new char[size]);
			f.readBytes(buf.get(), size);
			JsonObject& cfg = data.parseObject(buf.get());
			f.close();
			if (cfg.success()) {
				cfg.prettyPrintTo(Serial);
				Serial.print("\n");
				yield();
			}
		}
	}

	String readConfig(String field)
	{
		StaticJsonBuffer<JSONCONFIGSIZE>  data;
		File f = SPIFFS.open("/config.json", "r");
		if (f) {
			size_t size = f.size();
			if ( size == 0 ) {
				f.close();
				return "";
			}else{
				std::unique_ptr<char[]> buf (new char[size]);
				f.readBytes(buf.get(), size);
				JsonObject& cfg = data.parseObject(buf.get());
				if (!cfg.success()) {
					return "";
				}
			f.close();
			return cfg[field];
			}
		} else
		{
			return "";
		}
	}

	bool writeConfig(String field, String val)
	{
		StaticJsonBuffer<JSONCONFIGSIZE>  data;
		File f = SPIFFS.open("/config.json", "r");
		if ( (f) && (f.size() >0) )
		{
			size_t size = f.size();
			std::unique_ptr<char[]> buf (new char[size]);
			f.readBytes(buf.get(), size);
			JsonObject& cfg = data.parseObject(buf.get());
			if (cfg.success()) {
				cfg[field] = val;
				f.close();
				yield();
				f = SPIFFS.open("/config.json", "w");
				cfg.printTo(f);
				f.close();
				yield();
				return true;
			}
			else
			{
				Serial.println("Bad JSON :(");
				return false;
			}

		}
		 else
		{
			StaticJsonBuffer<JSONCONFIGSIZE> data;
			JsonObject& root = data.createObject();
			root[field] = val;
			yield();
			f = SPIFFS.open("/config.json", "w");
			root.printTo(f);
			f.close();
			yield();
			return true;
		}
	}

protected:


};
#endif

#endif // SPIFS_H
