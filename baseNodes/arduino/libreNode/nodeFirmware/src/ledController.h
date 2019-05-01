#ifndef LEDCONTROLLER
#define LEDCONTROLLER

#include "storage.h"

class ledController{
public:
    ledController(storage* s)
    {
      _storage        = s;
      _artnetRX       = 0;
      _sacnRX         = 0;
      _dmxSetup       = false;
      _bytesPerPixel  = 3;
      _frames         = 0;
      _dmxLastTime    = 0;
      setup();
    }

    void setup()
    {
      String ledhwType   = _storage->getLedConfig(LED_HW);
      _ledCount          = _storage->getLedConfig(LED_COUNT    ).toInt();
      _maxPower          = _storage->getLedConfig(LED_MAXBRIGHT).toInt();
      _maxBright         = _maxPower;
      _brightness        = _storage->getLedConfig(LED_BRIGHT   ).toFloat();
      _statusLedPin      = _storage->getLedConfig(STATUS_PIN   ).toInt();
      _underVoltProtect  = _storage->getLedConfig(UNDERVOLT_PROTECT) == ENABLED;
      _ledArray          =  new CRGB[_ledCount];


      Serial.println(String(F("Setting up led hardware... "))+ledhwType);
      if      (ledhwType == LED_WS2812)
      {
        Serial.print(String(F("Neopixel")));
        LEDS.addLeds<NEOPIXEL,LED_PIN>(_ledArray, _ledCount);
      }
      else if (ledhwType == LED_WS2801)
      {
        Serial.print(String(F("WS2801")));
        LEDS.addLeds<WS2801,LED_PIN,LED_CLOCK,BGR>(_ledArray, _ledCount);
      }
      else if (ledhwType == LED_APA102)
      {
        Serial.print(String(F("APA102")));
        LEDS.addLeds<APA102, LED_PIN,LED_CLOCK,BGR>(_ledArray, _ledCount);
      }
      else
      {
        Serial.print(String(F("DefaultNeopixel")));
        LEDS.addLeds<NEOPIXEL,LED_PIN>(_ledArray, _ledCount);
      }


      Serial.println(String(F(" ledcount:"))+String(_ledCount)+" brightness: "+String(_brightness)+" * "+String(_maxBright)+" = "+String(_maxBright*_brightness));
      FastLED.setBrightness(_maxBright*_brightness);
      _pixelsPerUniverse = 512 / _bytesPerPixel;
      _maxDMXData        = (_ledCount*_bytesPerPixel)+(_ledCount/_pixelsPerUniverse);

      #ifdef ADC_UNDERVOLT
      _vccRef = ESP.getVcc()/1000.0;
      if(_storage->getLedConfig(POWER_CALIBRATION) == ENABLED)
      {
        calibratePower();
      }
      #endif

      off();
      delay(50);
      _underVoltDimmer = 1.0;
      Serial.println(F("...Leds ok!"));
      yield();
      //setColor(200, 200, 0);
    }

    uint16_t  ledCount()        {return _ledCount;}
    uint8_t   maxPower()        {return _maxBright;}
    float     brightness()      {return _brightness;}
    float     undervoltDimmer() {return _underVoltDimmer;}

    void setBrightness(float b)
    {
      if(b>1)b = 1;
      if(b<0)b = 0;
      _brightness = b;
    }

    void setMaxPower(uint8_t b)
    {
      _maxBright = b;
    }

    void setColor(uint8_t r, uint8_t g, uint8_t b)
    {
        for(uint16_t i = 0 ; i < _ledCount ; i++)
          setPixelColor(i,r,g,b);
    }

    void progress(uint8_t p,CRGB c = CRGB(0,100,0))
    {
      if(p>100)p = 100;
      setColor(0,0,0);
      uint16_t lit = _ledCount*(p/100.0);
      for(int l = 0 ; (l < lit); l++)
        _ledArray[l] = c;
      show();
    }

    CRGB* pixel(uint16_t index)
    {
      if(index < _ledCount)
        return &_ledArray[index];
      else
        return &_ledArray[0];


    }

    void setPixelColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b,uint8_t w = 0)
    {
        if(index < _ledCount)
        {
          _updated = true;
          _ledArray[index].r=r;
          _ledArray[index].g=g;
          _ledArray[index].b=b;
        }
    }

    void showAsync()
    {
      _updated = true;
    }

    void off()
    {
      for(uint16_t i = 0 ; i < _ledCount ; i++)
        setPixelColor(i,0,0,0);
      FastLED.show();
      _updated = false;
    }

    void update()
    {
      if(_underVoltProtect)
        checkVCC();

      if(_dmxSetup)
      {
        while (_sacnEnabled   && readsACN  ()) {;}
        while (_artnetEnabled && readArtnet()) {;}
      }

      if(_updated)
      {
        show();
        _updated = false;
      }
      else
      {
        if( (millis() - _lastLedRefresh) > 2000)
          show();
      }

      uint32_t now = millis();
      if(_statusLedDecay > 0)
      {
        _statusLedDecay -= now - _lastUpdated;
        if(_statusLedDecay <=0)
          digitalWrite(_statusLedPin,HIGH);
      }
      _lastUpdated = now;
    }

    void show()
    {
      _frames++;
      FastLED.setBrightness(_maxBright*_brightness*_underVoltDimmer);
      FastLED.show();
      if(_underVoltProtect)
        checkVCC();
      _lastLedRefresh = millis();
    }

    uint16_t getsACNrx()
    {
      uint16_t r  = _sacnRX;
      _sacnRX     = 0;
      return r;
    }

    uint16_t getArtnetrx()
    {
      uint16_t r  = _artnetRX;
      _artnetRX   = 0;
      return r;
    }

    uint16_t getFrames()
    {
      uint16_t f  = _frames;
      _frames     = 0;
      return f;
    }

    LXWiFiArtNet* artnetInterface()
    {
      return _artNet0;
    }

    bool dmxInUse()
    {
      if(_dmxLastTime == 0)
        return false;
      uint32_t now = millis();
      return (now - _dmxLastTime) < 60000;
    }

    void initDMX()
    {
      _dmxSetup = true;
      if(_storage->getLedConfig(SACN_ENABLED) == ENABLED)
      { //SACN
        _sacnEnabled    = true;
        _sacnUniverse   = _storage->getLedConfig(SACN_UNIVERSE).toInt();
        _sacnChannel    = _storage->getLedConfig(SACN_CHANNEL).toInt();
        _sACN0 = new LXWiFiSACN();
        _sACN0->setUniverse(_sacnUniverse);
        _sUDP.beginMulticast(WiFi.localIP(), IPAddress(239,255,0,_sacnUniverse), _sACN0->dmxPort());
        Serial.println(String(F("sACN enabled,channel:"))+ String(_sacnChannel) + String(F(" universe: "))+String(_sacnUniverse));
      }
      else
        _sacnEnabled = false;

      if(_storage->getLedConfig(ARTNET_ENABLED) == ENABLED)
      { //ARTNET
        _artnetEnabled  = true;
        _artnetUniverse = _storage->getLedConfig(ARTNET_UNIVERSE).toInt();
        _artnetChannel  = _storage->getLedConfig(ARTNET_CHANNEL).toInt();
        Serial.println(String(F("Artnet enabled,channel:"))+ String(_artnetChannel) + String(F(" universes: ")));

        _artNet0 = new LXWiFiArtNet(WiFi.localIP(), WiFi.subnetMask(),&_dmxBuffer[0]);
        _artNet0->setUniverse(_artnetUniverse);  //setUniverse for LXArtNet class sets complete Port-Address
        Serial.println(_artnetUniverse);

        if( _maxDMXData >512)
        {
          _artNet1 = new LXWiFiArtNet(WiFi.localIP(), WiFi.subnetMask(),&_dmxBuffer[0]);
          _artNet1->setUniverse(_artnetUniverse+1);
          Serial.println(_artnetUniverse+1);
        }
        if( _maxDMXData >1024)
        {
          _artNet2 = new LXWiFiArtNet(WiFi.localIP(), WiFi.subnetMask(),&_dmxBuffer[0]);
          _artNet2->setUniverse(_artnetUniverse+2);
          Serial.println(_artnetUniverse+2);
        }
        if( _maxDMXData >1536)
        {
          _artNet3 = new LXWiFiArtNet(WiFi.localIP(), WiFi.subnetMask(),&_dmxBuffer[0]);
          _artNet3->setUniverse(_artnetUniverse+3);
          Serial.println(_artnetUniverse+3);
        }
        if( _maxDMXData >2048)
        {
          _artNet4 = new LXWiFiArtNet(WiFi.localIP(), WiFi.subnetMask(),&_dmxBuffer[0]);
          _artNet4->setUniverse(_artnetUniverse+4);
          Serial.println(_artnetUniverse+4);
        }


        if( _storage->getLedConfig(ARTNET_ANNOUNCE) == ENABLED)
        {
          Serial.println(F("Artnet poll enabled"));
          _aUDP.begin(_artNet0->dmxPort());
          String id = _storage->getNodeConfig(NODE_ID);
          String id0 = id+"-0";
          strcpy(_artNet0->longName(), id0.c_str());
          _artNet0->send_art_poll_reply(&_aUDP);
          yield();

          if( _maxDMXData >512)
          {
            String id1 = id+"-1";
            strcpy(_artNet1->longName(), id1.c_str());
            _artNet1->send_art_poll_reply(&_aUDP);
            yield();
          }
          if( _maxDMXData >1024)
          {
            String id2 = id+"-2";
            strcpy(_artNet2->longName(), id2.c_str());
            _artNet2->send_art_poll_reply(&_aUDP);
            yield();
          }
          if( _maxDMXData >1536)
          {
            String id3 = id+"-3";
            strcpy(_artNet3->longName(), id3.c_str());
            _artNet3->send_art_poll_reply(&_aUDP);
            yield();
          }
          if( _maxDMXData >2048)
          {
            String id4 = id+"-4";
            strcpy(_artNet4->longName(), id4.c_str());
            _artNet4->send_art_poll_reply(&_aUDP);
            yield();
          }
        }
     }
     else
     {
      _artnetEnabled = false;
     }
    }

    void checkVCC()
  	{

  		float currentVolts = ESP.getVcc()/1000.0;

      if( ((currentVolts - _vccRef) > 0.2) && (_underVoltDimmer == 1.0) )
      {
        if(_maxPower > _maxBright)
        {
          calibratePower();
        }

        _vccRef = currentVolts;
      }

      if(_underVoltDimmer > 1)
        _underVoltDimmer = 1;

      float undervolt = _vccRef - currentVolts;

  		if((undervolt < 0.08) && (_underVoltDimmer < 1.0))
  		{
        _underVoltDimmer += 0.0001;
  			//Serial.println("Voltage is good, rising factor: "+String(m_ledHardware->underVoltDimm())+" Result: "+ String(currentVolts));
  		}
      else if(undervolt >= 0.3)
      {
        _underVoltDimmer = 0.02;
  			show();
  		}
      else if(undervolt >= 0.08)
      {
        _underVoltDimmer -= undervolt/2;
        if(_underVoltDimmer < 0)
          _underVoltDimmer = 0.01;
  			show();
  		}
  	}

    void calibratePower()
    {
      #ifdef ADC_UNDERVOLT
      Serial.println(F("Calibrate power.."));
      yield();
      off();
      delay(50);
      float vccRef = ESP.getVcc()/1000.0;
      float underv = 0;
      for(_maxBright = 0 ; _maxBright < _maxPower ; _maxBright++)
      {
        setColor(255,255,255);
        FastLED.setBrightness(_maxBright);
        FastLED.show();
        delay(1);
        underv = vccRef - (ESP.getVcc()/1000.0);
        if(underv > 0.01)
        {
          off();
          if(_maxBright > 40) _maxBright -= 10;
          break;
        }
      }
      FastLED.setBrightness(_maxBright);
      Serial.println(String(F("Done, MaxPower:"))+String(_maxBright)+String(F(" vloss:"))+String(underv));
      delay(50);
      #endif
    }

protected:
  storage*  _storage;
  uint16_t  _ledCount;
  CRGB*     _ledArray;
  uint8_t   _maxBright;
  uint8_t   _maxPower;
  float     _brightness;
  bool      _underVoltProtect;
  float     _underVoltDimmer;
  float     _vccRef;
  uint8_t   _statusLedPin;


  int       _statusLedDecay;
  uint32_t  _lastUpdated;
  uint32_t  _lastLedRefresh;
  uint16_t  _frames;
  bool      _updated;



//Artnet & sacn
  bool          _dmxSetup;
  uint32_t      _dmxLastTime;
  uint8_t       _bytesPerPixel;
  uint8_t       _pixelsPerUniverse;
  bool          _sacnEnabled;
  uint16_t      _sacnUniverse;
  uint16_t      _sacnChannel;
  bool          _artnetEnabled;
  uint16_t      _artnetUniverse;
  uint16_t      _artnetChannel;
  uint8_t       _dmxBuffer[SACN_BUFFER_MAX];
  uint16_t      _maxDMXData;
  uint16_t      _artnetRX;
  uint16_t      _sacnRX;
  LXWiFiArtNet* _artNet0;
  LXWiFiArtNet* _artNet1;
  LXWiFiArtNet* _artNet2;
  LXWiFiArtNet* _artNet3;
  LXWiFiArtNet* _artNet4;
  LXWiFiSACN*   _sACN0;

  WiFiUDP _aUDP;
  WiFiUDP _sUDP;




  bool readsACN()
  {
    bool updated = false;
    if(_sACN0->readDMXPacket(&_sUDP) == RESULT_DMX_RECEIVED)
    {
      _sacnRX++;
      _statusLedDecay = 20;
      digitalWrite(_statusLedPin,LOW);
      uint16_t s_slots = _sACN0->numberOfSlots();
      for(int i = 1; i < s_slots ; i+=3)
      {
        if(i>_maxDMXData) break;
          setPixelColor(i/_bytesPerPixel, _sACN0->getSlot(i), _sACN0->getSlot(i+1), _sACN0->getSlot(i+2));
      }
      updated = true;
      _dmxLastTime = millis();
    }
    return updated;
  }

  bool readArtnet()
  {
    bool updated = false;
    uint16_t packetSize = _aUDP.parsePacket();
    while(packetSize)
    {
      _artnetRX++;
      _statusLedDecay = 20;
      digitalWrite(_statusLedPin,LOW);

      packetSize = _aUDP.read(_dmxBuffer, SACN_BUFFER_MAX);
    //U0
      if(_artNet0->readDMXPacketContents(&_aUDP, packetSize) == RESULT_DMX_RECEIVED)
      {
        for(int i = _artnetChannel; i < _artNet0->numberOfSlots() - 3 ; i+=_bytesPerPixel)
        {
          uint16_t p = i/_bytesPerPixel;
          setPixelColor(p, _artNet0->getSlot(i), _artNet0->getSlot(i+1), _artNet0->getSlot(i+2));
        }
        updated = true;
      }
    //U1
      else if((_maxDMXData>512) && (_artNet1->readDMXPacketContents(&_aUDP, packetSize) == RESULT_DMX_RECEIVED))
      {
        for(int i = 1; i < _artNet1->numberOfSlots() - 3 ; i+=_bytesPerPixel)
        {
          uint16_t p = (i/_bytesPerPixel) + _pixelsPerUniverse;
          setPixelColor(p, _artNet1->getSlot(i), _artNet1->getSlot(i+1), _artNet1->getSlot(i+2));
        }
        updated = true;
    //U2
    }
    else if((_maxDMXData>1024) && (_artNet2->readDMXPacketContents(&_aUDP, packetSize) == RESULT_DMX_RECEIVED))
    {
      for(int i = 1; i < _artNet2->numberOfSlots() - 3 ; i+=_bytesPerPixel)
      {
        uint16_t p = (i/_bytesPerPixel) + (_pixelsPerUniverse*2);
        setPixelColor(p, _artNet2->getSlot(i), _artNet2->getSlot(i+1), _artNet2->getSlot(i+2));
      }
      updated = true;
    //U3
    }
     else if((_maxDMXData>1536) && (_artNet3->readDMXPacketContents(&_aUDP, packetSize) == RESULT_DMX_RECEIVED))
      {
        for(int i = 1; i < _artNet3->numberOfSlots() - 3 ; i+=_bytesPerPixel)
        {
          uint16_t p = (i/_bytesPerPixel) + (_pixelsPerUniverse*3);
          setPixelColor(p, _artNet3->getSlot(i), _artNet3->getSlot(i+1), _artNet3->getSlot(i+2));
        }
        updated = true;
    //U4
    }
    else if((_maxDMXData>2048) && (_artNet4->readDMXPacketContents(&_aUDP, packetSize) == RESULT_DMX_RECEIVED))
     {
       for(int i = 1; i < _artNet4->numberOfSlots() - 3 ; i+=_bytesPerPixel)
       {
         uint16_t p = (i/_bytesPerPixel) + (_pixelsPerUniverse*4);
         setPixelColor(p, _artNet4->getSlot(i), _artNet4->getSlot(i+1), _artNet4->getSlot(i+2));
       }
       updated = true;
    }
    yield();
    packetSize = _aUDP.parsePacket();
  }
  if(updated)
    _dmxLastTime = millis();
  return updated;
 }
};

#endif // LEDCONTROLLER
