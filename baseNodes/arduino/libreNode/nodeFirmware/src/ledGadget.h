#ifndef LEDGADGET
#define LEDGADGET

#include "ledController.h"
#include "ledAnimations.h"
#include "nodeComponent.h"

class ledGadget : public nodeComponent
{
public:
  ledGadget(String id,storage* s, ledController* lc, int startLed = 0, int count = 0, bool reversed = false) :
    nodeComponent(id,s)
  {
    _ledController = lc;

    if(count == 0)
        count = lc->ledCount();

    for(uint16_t i = startLed ; i < startLed+count ; i++)
        if((i < lc->ledCount()) && (i >= 0)) _leds.push_back(lc->pixel(i));

    if(reversed)
      _leds = invertLedOrder(_leds);
    addTopic(F("setColor"));
    addTopic(F("setRandomColor"));
    addTopic(F("setEffect"));
    addTopic(F("setBright"));
    addTopic(F("defaultState"));
    addTopic(F("off"));
  }

  void setupComponent()
  {
    defaultState();
  }

  virtual void initConfig(JsonObject& cfg)
  {
    cfg["r"]      = "15";
    cfg["g"]      = "30";
    cfg["b"]      = "70";
    cfg[F("effect")] = E_COLOR;
    cfg[F("bright")] = F("1.0");
    cfg["v"]         = "2";
  }

  virtual void loadConfig(JsonObject& cfg)
  {
    int r = cfg["r"].as<int>();
    int g = cfg["g"].as<int>();
    int b = cfg["b"].as<int>();
    _defaultColor  = CRGB(r,g,b);
    _color         = _defaultColor;
    _defaultEffect = cfg[F("effect")].as<String>();
    yield();
  }

  virtual void getJsonData(JsonObject& data)
  {
    JsonObject& d      = data.createNestedObject(_id);
    JsonObject& col    = d.createNestedObject(F("color"));
    col["r"]           = _color.r;
    col["g"]           = _color.g;
    col["b"]           = _color.b;
    d[F("effect")]     = _currentEffect;
    d[F("bright")]     = _ledController->brightness();
  }

  virtual bool readTopic(char* topic, byte* payload, unsigned int length)
  {
    String t(topic);
    t = t.substring(t.lastIndexOf("/")+1);
    //Serial.println("Topic :"+t + " data: "+String((char*)payload));
    if     (t == F("off"))           {fade();}
    else if(t == F("defaultState"))  {defaultState();}
    else if(t == F("setBright"))     {setBrightness(String((char*)payload).toFloat());}
    else if(t == F("setEffect"))     {parseEffect((char *)payload);}
    else if(t == F("setRandomColor")){randomColor();}
    else if(t == F("setColor"))
    {
      StaticJsonBuffer<JSONCONFIGSIZE> data;
      JsonObject& cfg = data.parseObject(payload);
      if(cfg.success())
      {
        uint8_t r = cfg["r"];
        uint8_t g = cfg["g"];
        uint8_t b = cfg["b"];
        fadeToColor(CRGB(r,g,b));
      }
    }
    else
      return false;
    return true;
  }

  void updateComponent()
  {
    if(_ledController->dmxInUse())
      return;
    if(animate())
    {
      _lastTimeRendered = millis();
      _ledController->showAsync();
      if((_lastEffect != _currentEffect) || (_lastColor  != _color))
      {
        _lastEffect = _currentEffect;
        _lastColor  = _color;
        pollData();
      }

    }
  }

  void show()
  {
    if(_ledController->dmxInUse())
      return;
    _ledController->show();
  }

  CRGB getPixelColor(uint16_t index)
  {
    if(index < _leds.size())
      return CRGB();
    CRGB r;
    r.r = _leds[index]->r;
    r.g = _leds[index]->g;
    r.b = _leds[index]->b;
    return r;
  }


  void setColor(CRGB ncolor)
  {
    if(_ledController->dmxInUse())
      return;
    for(uint i = 0 ; i < _leds.size() ;i++)
    {
      _leds[i]->r = ncolor.r;
      _leds[i]->g = ncolor.g;
      _leds[i]->b = ncolor.b;
    }
  }

  void setColor(uint8_t r, uint8_t g, uint8_t b)
  {
      setColor(CRGB(r,g,b));
  }

  void setBrightness(float b)
  {
    _ledController->setBrightness(b);
    publish("bright",String(_ledController->brightness()));
  }


  void off          ()              {setColor(0,0,0);}
  void white        ()              {setColor(200,200,200);}
  void red          (int r = 200)   {setColor(r,0,0);}
  void green        (int g = 200)   {setColor(0,g,0);}
  void blue         (int b = 200)   {setColor(0,0,b);}
  void randomColor  ()              {fadeToColor(CRGB(rand()%255,rand()%255,rand()%255));}

  void clearEffects()
  {
    off();
    resetCounters();
    _currentEffect = E_OFF;
    show();
  }

  void parseEffect(char* jsonData)
  {
    StaticJsonBuffer<300>  data;
    JsonObject& c = data.parseObject(jsonData);
    if (!c.success())
    {
      Serial.println(F("json failed!"));
      return;
    }
    String t = c[F("effect")];

    if     (t == E_CLIGHT)   {chaoticLight();}
    else if(t == E_FADE)     {fade();}
    else if(t == E_GLOW)     {glow();}
    else if(t == E_CYLON)    {cylon();}
    else if(t == E_RAINBOW)  {rainbow();}
    else if(t == E_SPARKS)   {sparks();}
    else if(t == E_CHRISTMAS){christmas();}
    else if(t == E_PROGRESS) {progress(c["val"].as<int>());}
    else if(t == E_RANDOMCOLOR){randomColor();}
    else if(t == E_RANDOM)     {_currentEffect = E_RANDOM;}
    else if(t == E_FIRE)       {_currentEffect = E_FIRE;}
    else if(t == E_OFF)        {fade();}
    else if(t == E_COLOR)
    {
      int r = c["r"].as<int>(); int g = c["g"].as<int>(); int b = c["b"].as<int>();
      fadeToColor(CRGB(r,g,b));
    }
    else if(t == E_FLASH)
    {
      int r = c["r"].as<int>(); int g = c["g"].as<int>(); int b = c["b"].as<int>();
      int times = c[F("times")].as<int>(); int l = c[F("length")].as<int>();
      if((r==0)&&(g==0)&&(b==0))  instantFlash();
      else if((times>0) && (l >0))instantFlash(CRGB(r,g,b),times,l);
      else                        instantFlash(CRGB(r,g,b));
    }
    else if(t == E_STROBE)
    {
      int r = c["r"].as<int>(); int g = c["g"].as<int>(); int b = c["b"].as<int>();
      int hz = c["hz"].as<int>();
      if(((r==0)&&(g==0)&&(b==0))) strobe();
      else if(hz>0)                strobe(CRGB(r,g,b),hz);
      else                         strobe(CRGB(r,g,b));
    }

  }

  void setEffect(String e)
  {
    clearEffects();
    _currentEffect = e;
  }

  void defaultState()
  {
    _currentEffect = _defaultEffect;
    _color         = _defaultColor;
  }

  String effect()             {return _currentEffect;}
//efectos

  virtual void fade()         {resetCounters(); _currentEffect = E_FADE;   }
  virtual void sparks()       {resetCounters(); _currentEffect = E_SPARKS; }
  virtual void cylon()        {resetCounters(); _currentEffect = E_CYLON  ;paintCylon(_leds,_counters,0);}
  virtual void rainbow()      {resetCounters(); _currentEffect = E_RAINBOW;paintRainbow(_leds,_counters,0);}
  virtual void chaoticLight() {resetCounters(); _currentEffect = E_CLIGHT ;paintChaoticLight(_leds);}
  virtual void christmas()    {resetCounters(); _currentEffect = E_CHRISTMAS;paintChristmas(_leds,_counters,0);}

  virtual void glow(CRGB c = CRGB(0,0,0))
  {
    resetCounters();
    _currentEffect = E_GLOW;
    if(c != CRGB(0,0,0))
      _color = c;
  }

  virtual void strobe(CRGB c = CRGB(200,200,200), uint8_t Hz = 10)
  {
      resetCounters();
      _currentEffect = E_STROBE;
      _counters.c0   = 1000/Hz;
      _color         = c;
      _lastStrobe = millis();
  }

  virtual void progress(uint8_t p)
  {
    _currentEffect = E_PROGRESS;
    if(p>100)p = 100;
    setColor(0,0,0);
    uint16_t lit = _leds.size()*(p/100.0);
    for(int l = 0 ; (l < lit); l++)
      *_leds[l] =CRGB(0,150,0);
    show();
  }

  virtual void fadeToColor(CRGB c)
  {
    bool alreadyColor = true;
    for(uint16_t l = 0 ; l < _leds.size() ; l++)
      if(*_leds[l] != c) alreadyColor = false;

    if(alreadyColor == true)  {_currentEffect = E_COLOR;return;}
    resetCounters();
    _currentEffect  = E_FADETOCOLOR;
    _color          = c;
  }

  virtual void instantFade()
  {
    fade();
    while (_currentEffect != E_OFF)
    {
      animateFade();
      _ledController->show();
      delay(10);
    }
  }

  virtual void instantFadeToColor(CRGB c)
  {
    fadeToColor(c);
    while (_currentEffect != E_COLOR)
    {
      animateFadeToColor();
      _ledController->show();
      delay(10);
    }
  }

  virtual void instantFlash(CRGB color = CRGB(100,100,0), uint times = 2,uint length = 250)
  {
    saveState();
    for(uint t = 0 ; t<times ; t++)
    {
      setColor(color);
      show();
      delay(length);
      instantFade();
    }
    restoreState();
  }

  void saveState()
  {
      _scounters = _counters;
      _seffect   = _currentEffect;
      _scolor    = _color;
  }

  void restoreState()
  {
    _counters       = _scounters;
    _currentEffect  = _seffect;
    _color          = _scolor;
  }

protected:
  ledController*     _ledController;
  std::vector<CRGB*> _leds;

  CRGB      _color;
  CRGB      _defaultColor;


  uint32_t  _lastTimeRendered;
  uint32_t  _lastStrobe;

  animationCounters _scounters;
  CRGB              _scolor;
  String            _seffect;

  CRGB   _lastColor;
  String _lastEffect;

  animationCounters _counters;

  String _currentEffect;
  String _defaultEffect;


  void resetCounters()
  {
    _counters = animationCounters();
  }

  virtual bool animate()
  {
    uint32_t now = millis();
    uint16_t steps = now - _lastTimeRendered;
    random16_add_entropy(rand());

    if      ( _currentEffect == E_FADE)       animateFade       ();
    else if ( _currentEffect == E_FADETOCOLOR)animateFadeToColor();
    else if ( _currentEffect == E_FIRE)       animateFire       ();
    else if ( _currentEffect == E_RANDOM)     animateRandom     ();
    else if ( _currentEffect == E_GLOW)       animateGlow       (steps);
    else if ( _currentEffect == E_CYLON)      animateCylon      (steps);
    else if ( _currentEffect == E_RAINBOW)    animateRainbow    (steps);
    else if ( _currentEffect == E_SPARKS)     paintSparks       (_leds,steps);
    else if ( _currentEffect == E_CLIGHT)     paintChaoticLight (_leds);
    else if ( _currentEffect == E_COLOR)      {fadeToColor(_color);return false;}
    else if ( _currentEffect == E_STROBE)     {animateStrobe()    ;return false;}
    else if ( _currentEffect == E_CHRISTMAS)  paintChristmas    (_leds,_counters,steps);
    else
      return false;

    return true;
  }

//efectos
  virtual void animateFire()
  {
    paintFire2012(_leds);
  }

  virtual void animateRainbow(uint16_t steps)
  {
    paintRainbow(_leds,_counters,steps);
  }

  virtual void animateCylon(uint16_t steps)
  {
    paintCylon(_leds,_counters,steps);
  }

  virtual void animateChristmas(uint16_t steps)
  {
    paintChristmas(_leds,_counters,steps);
  }

  virtual void animateGlow(uint8_t steps = 1)
  {
    float f = (200 - _counters.c0)/150.0;
    if(_counters.c1 == 0)
    {
      _counters.c0+=steps/4;
      if(_counters.c0 >=200.0)
        _counters.c1 = 1;
    }
    else
    {
      _counters.c0-=steps/4;
      if(_counters.c0 <=50)
        _counters.c1 = 0;
    }
    CRGB c;
    if(f >  1.0) f = 1.0;
    if(f <  0.05) f = 0.05;
    c.r = _color.r*f;
    c.g = _color.g*f;
    c.b = _color.b*f;
    setColor(c);
  }

  virtual void animateFade()
  {
    bool allOff = true;
    for(uint16_t i = 0 ; i < _leds.size() ; i++)
    {
      _leds[i]->r *= 0.8;
      _leds[i]->g *= 0.8;
      _leds[i]->b *= 0.8;
      if(*_leds[i] != CRGB(0,0,0))
        allOff = false;
    }

    if(allOff)
    {
      _currentEffect = E_OFF;
    }
  }

  virtual void animateFadeToColor()
  {
    bool allDone = true;
    for(uint16_t l = 0 ; l < _leds.size() ; l++)
    {
      _leds[l]->r += (_color.r - _leds[l]->r)/5;
      _leds[l]->g += (_color.g - _leds[l]->g)/5;
      _leds[l]->b += (_color.b - _leds[l]->b)/5;

      if(abs(_color.r - _leds[l]->r) < 5) _leds[l]->r = _color.r;
      if(abs(_color.g - _leds[l]->g) < 5) _leds[l]->g = _color.g;
      if(abs(_color.b - _leds[l]->b) < 5) _leds[l]->b = _color.b;

      if(CRGB(*_leds[l]) != _color) allDone = false;
    }

    if(allDone)
      _currentEffect = E_COLOR;
  }

  virtual void animateStrobe()
  {
    uint32_t now = millis();
    //Serial.println("Now:" + String(now) + " t: " + String(now - _lastStrobe) + " c: " + String(_leds[0]->r)+ " cs: " + String(_color.r));
    if( (now - _lastStrobe) > _counters.c0)
    {
      setColor(_color);
      _lastStrobe = now - ((now - _lastStrobe - _counters.c0)%_counters.c0);
      show();
    }
    else if( (now - _lastStrobe) > (_counters.c0/2))
    {
      if(*_leds[0] != CRGB(0,0,0))
      {
        off();
        show();
      }
    }
  }

  virtual void animateRandom()
  {
    instantFadeToColor(CRGB(rand()%255,rand()%255,rand()%255));
    _currentEffect = E_RANDOM;
  }
};

class ledBar : public ledGadget
{
public:
ledBar(String id,storage* s, ledController* lc, int startLed = 0, int count = 0, bool reversed = false) :
  ledGadget(id,s,lc,startLed,count,reversed)
  {

  }
protected:

};

class ledMatrix : public ledGadget
{
public:
ledMatrix(String id,storage* s, ledController* lc) :
  ledGadget(id,s,lc)
  {

  }
  virtual void initConfig(JsonObject& cfg)
  {
    ledGadget::initConfig(cfg);
    cfg[F("xTiles")]    = 3;
    cfg[F("yTiles")]    = 1;
    cfg[F("yTileSize")] = 16;
    cfg[F("xTileSize")] = 16;
  }

  virtual void loadConfig(JsonObject& cfg)
  {
    ledGadget::loadConfig(cfg);
    _xTiles =     cfg[F("xTiles")];
    _yTiles =     cfg[F("yTiles")];
    _tileYsize =  cfg[F("yTileSize")];
    _tileXsize =  cfg[F("xTileSize")];
  }

  CRGB* pixel(uint16_t x, uint16_t y)
  {
    return getRow(x)[y];
  }

  std::vector<CRGB*> getRow(uint16_t r)
  {
  std::vector<CRGB*> result;

  for(uint16_t tX = 0 ; tX < _xTiles ; tX++)
    for(uint16_t x = 0 ; x < _tileXsize ; x++)
    {
      uint16_t tY = r / _tileYsize;
      uint16_t tileOffset = (tY*_tileYsize*_tileXsize*_xTiles) + (_tileYsize*_tileXsize*tX);
      uint16_t index = tileOffset;
      uint16_t y = 0;
      if(tY) y = r%_tileYsize;
      else   y = r;

      if(x%2) index += ((x+1)*_tileYsize)-y-1;
      else    index += (x*_tileYsize)+y;

      if(index < _ledController->ledCount()) result.push_back(_leds[index]);
      else                                   result.push_back(_leds.back());
    }
  return result;
  }

  std::vector<CRGB*> getCol(uint16_t c)
  {
    std::vector<CRGB*> result;
    for(uint16_t tY = 0 ; tY < _yTiles ; tY++)
      for(uint16_t y = 0 ; y < _tileYsize ; y++)
      {
        uint16_t tX = c / _tileXsize;
        uint16_t tileOffset = (tY*_tileYsize*_tileXsize*_xTiles) + (_tileYsize*_tileXsize*tX);
        uint16_t index = tileOffset;
        uint16_t x = 0;

        if(tX) x = c%_tileXsize;
        else   x = c;
        index += (x*_tileYsize);
        if(x%2)  index += y;
        else     index += _tileYsize - y-1;
        if(index < _ledController->ledCount()) result.push_back(_leds[index]);
        else                                   result.push_back(_leds.back());
      }
    return result;
  }

  uint16_t width()
  {
      return _tileXsize * _xTiles;
  }

  uint16_t height()
  {
      return _tileYsize * _yTiles;
  }
  virtual bool readTopic(char* topic, byte* payload, unsigned int length)
  {
    if(ledGadget::readTopic(topic,payload,length)) return true;
    else return false;
  }

protected:
  uint16_t _tileXsize;
  uint16_t _tileYsize;
  uint8_t  _xTiles;
  uint8_t  _yTiles;

  virtual void animateRainbow(uint16_t steps)
  {
    std::vector<CRGB*> row = getRow(0);
    paintRainbow(row,_counters,steps);
    for(uint16_t r = 1 ; r < height() ; r++)
    {
      std::vector<CRGB*> nrow = getRow(r);
      copyLeds(row,nrow);
    }
  }

  virtual void animateCylon(uint16_t steps)
  {
    std::vector<CRGB*> row = getRow(0);
    paintCylon(row,_counters,steps);
    for(uint16_t r = 1 ; r < height() ; r++)
    {
      std::vector<CRGB*> nrow = getRow(r);
      copyLeds(row,nrow);
    }
  }

  virtual void animateFire()
  {
    for(uint16_t c = 0 ;c < width() ; c++)
    {
      //std::vector<CRGB*> col = invertLedOrder(getCol(c));
      std::vector<CRGB*> col = getCol(c);
      paintFire2012(col);
    }
  }
};

#endif
