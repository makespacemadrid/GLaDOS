#ifndef SPIFS_H
#define SPIFS_H

#define JSONCONFIGSIZE  200
#define JSONAUTHSIZE    500

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
  }

  void initSettings()
  {
    StaticJsonBuffer<JSONCONFIGSIZE> data;
    JsonObject& root = data.createObject();
    File f;
    root["id"]            = "unamedNode";
    root["nodeType"]      = "unconfiguredNode";
    root["overmind"]      = "192.168.10.10";
    root["authNode"]      = "true";
    root["authJSONUrl"]   = "192.168.10.10:8000/userAuth.json";

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
        if(cfg["id"] != "")
  			   return true;
        else
        {
          Serial.println("Found json but no id field");
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

  void updateUserAuth()
  {
    StaticJsonBuffer<JSONAUTHSIZE>  data;
    int localVersion = 0;
		File f = SPIFFS.open("/userAuth.json", "r");
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
  			Serial.print(" Auth Config from SPIFFS loaded: ");
  			Serial.print(cfg.size());
  			Serial.println(" records found.");
        String version = cfg["version"];
        if(version != "")
  			   localVersion = version.toInt();
        else
        {
          Serial.println("Found json but no version field");
        }
			}
		} else
		{
			Serial.println("No authConfig File found!");
		}
    Serial.print("local auth version: ");Serial.println(localVersion);

    //String url = readConfig("authJSONUrl");
    String url = "http://successbyfailure.org/userAuth.json";
    Serial.println("Fetching last userAuth: "+url);
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
          f = SPIFFS.open("/userAuth.json", "w");
          cfg.printTo(f);
          cfg.prettyPrintTo(Serial);
          f.close();
          yield();
        }
      }
      else
      {
        Serial.println("BAD JSON");
      }
    }
    else
    {
      Serial.println("Cannot get remote userAuth");
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
