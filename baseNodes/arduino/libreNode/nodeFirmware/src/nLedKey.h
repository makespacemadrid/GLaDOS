#ifndef LEDKEY
#define LEDKEY

#include "nLedBar.h"
#include "nodeComponent.h"

class ledKeyNode : public ledBarNode
{
public:
  ledKeyNode(storage* s) : ledBarNode(s),
    _button("button",s),
    _topLeds   (String(F("tLeds")),s,&_ledController,0,5),
    _rightLeds (String(F("rLeds")),s,&_ledController,5,4),
    _bottomLeds(String(F("bLeds")),s,&_ledController,9,5),
    _leftLeds  (String(F("lLeds")),s,&_ledController,15,4)
  {
    addComponent(&_button);
    _gameMode = false;

    _subscriptions.push_back(String(F("game/colorGame/#")));
  }

  void setupNode()
  {
    _leds.rainbow();
/*
    for(int i = 0 ; i < 5 ; i++)
    {
      _leds.instantFade();
      _topLeds.setColor(CRGB(rand()%255,rand()%255,rand()%255));
      _leds.show();
      _topLeds.instantFade();
      _bottomLeds.setColor(CRGB(rand()%255,rand()%255,rand()%255));
      _leds.show();
      _bottomLeds.instantFade();
      _rightLeds.setColor(CRGB(rand()%255,rand()%255,rand()%255));
      _leds.show();
      _rightLeds.instantFade();
      _leftLeds.setColor(CRGB(rand()%255,rand()%255,rand()%255));
      _leds.show();
      _leftLeds.instantFade();
    }
*/
  }

  void nodeLoop()
  {

    if(_gameMode)
    {
      if(_button.pressed())
      {
        _pushTime = millis();
        String t = F("game/colorGame/selected");
        mqttPub p(t,_id);
        publishMQTT(p);
      }
      else if(_button.status())
      {
        if(millis() - _pushTime > 5000)
        {
            _pushTime = millis();
            String t = F("game/colorGame/stop");
            String d = "";
            publishMQTT(t,d);
        }
        else
          _leds.progress((millis() - _pushTime)/5000.0*100);
      }


      return;
    }

    if(_button.pressed())
    {
      _pushTime = millis();
      _leds.saveState();
      _leds.setColor(0,150,0);
    }
    else if(_button.status())
    {
      if(millis() - _pushTime > 5000)
      {
          _pushTime = millis();
          String t = F("game/colorGame/start");
          String d = "";
          publishMQTT(t,d);
          _gameMaster = true;
      }
      else
        _leds.progress((millis() - _pushTime)/5000.0*100);
    }
    else if(_button.released())
    {
      _leds.restoreState();
      _leds.instantFlash(CRGB(0,200,0),1);
    }
  }

  bool readTopicNode(char* topic, byte* payload, unsigned int length)
  {
    String t = topic;

    if(t == String(F("game/colorGame/start")))
    {
        startColorGame();
    }
    else if(t == String(F("game/colorGame/stop")))
    {
      _gameMaster = false;
      stopGame();
    }
    else if(t == String(F("game/colorGame/selected")))
    {
      if(_gameMaster)
      {
        _roundProgress++;
        char q[length+1];
        for(uint8_t i = 0 ; i < length+1 ; i++)
          q[i] = 0;
        strncpy(q,(char*)payload,length);
        String s((char*)q);
        if(s == _roundWinner)
        {
          _leds.instantFlash(CRGB(0,255,0),1,500);
          _roundProgress++;
          runGame();
        }
        else
        {
          _leds.instantFlash(CRGB(250,0,0),1,500);
          String t = F("game/colorGame/stop");
          String d = "";
          publishMQTT(t,d);
        }
      }
    }
    else return false;
    return true;
  }

protected:
  pushButton _button;
  bool       _gameMode;
  bool       _gameMaster;
  bool       _gameLevelDone;
  uint32_t   _pushTime;
  uint8_t    _gameLevel;
  std::vector<CRGB>    _gameColors;
  String              _roundWinner;
  uint16_t            _roundProgress;
  ledBar     _topLeds;
  ledBar     _rightLeds;
  ledBar     _bottomLeds;
  ledBar     _leftLeds;

  void startColorGame()
  {
    _leds.saveState();
    _gameMode      = true;
    _gameLevel     = 0;
    _roundProgress = 0;
    _gameLevelDone = false;
    _leds.instantFlash(CRGB(155,0,155),1,1000);
    _leds.setEffect(E_GAME);

    if(_gameMaster == true)
    {
      _gameColors.push_back(CRGB(rand()%255,rand()%255,rand()%255));
      _gameColors.push_back(CRGB(rand()%255,rand()%255,rand()%255));
      _gameColors.push_back(CRGB(rand()%255,rand()%255,rand()%255));
      runGame();
    }
  }

  void stopGame()
  {
    _gameMode = false;
    _leds.restoreState();
    while(_gameColors.size())
    {
      _gameColors.erase(_gameColors.begin());
    }
  }

  void runGame()
  {
//    publishMQTT(String(F("node/Del/led/off")),"");
//    publishMQTT(String(F("node/Esc/led/off")),"");
//    publishMQTT(String(F("node/Shift/led/off")),"");
    if(_roundProgress >= _gameColors.size())
    {
      _gameColors.push_back(CRGB(rand()%255,rand()%255,rand()%255));
      _roundProgress = 0;
      _gameLevel++;
    }

    if(_roundProgress == 0)
    {
      _leds.setColor(200,200,200);
      _leds.show();
      delay(1000);
      _leds.instantFade();
      delay(1000);
      for(auto c : _gameColors)
      {
          _leds.instantFlash(c,1,1000);
          delay(500);
      }
    }
//
    StaticJsonBuffer<JSONCONFIGSIZE>   data;
    JsonObject& cfg = data.createObject();
    cfg["r"] = _gameColors[_roundProgress].r;
    cfg["g"] = _gameColors[_roundProgress].g;
    cfg["b"] = _gameColors[_roundProgress].b;
    String z;
    cfg.printTo(z);
    int sel = rand()%3;
    if(sel == 0)
    {
      publishMQTT(String(F("node/Del/led/off")),z);
      publishMQTT(String(F("node/Esc/led/off")),z);
      publishMQTT(String(F("node/Shift/led/setColor")),z);
     _roundWinner = F("Shift");
    }
    else if(sel == 1)
    {//setRandomColor
      publishMQTT(String(F("node/Del/led/off")),z);
      publishMQTT(String(F("node/Esc/led/setColor")),z);
      publishMQTT(String(F("node/Shift/led/off")),z);
     _roundWinner = F("Esc");
  }
    else if(sel == 2)
    {
      publishMQTT(String(F("node/Del/led/setColor")),z);
      publishMQTT(String(F("node/Esc/led/off")),z);
      publishMQTT(String(F("node/Shift/led/off")),z);
     _roundWinner = F("Del");
    }
  }


  void mqttConected()
  {
    baseNode::mqttConected();
    _leds.defaultState();
  }

  void mqttDisconnected()
  {
    baseNode::mqttDisconnected();
    _leds.setColor(0,0,50);
    _leds.glow();
  }

  void wifiDisconnected()
  {
    baseNode::wifiDisconnected();
    _leds.setColor(50,0,0);
    _leds.glow();
  }

  void wifiConnected()
  {
    _leds.setColor(0,0,50);
    _leds.glow();
    _leds.instantFlash(CRGB(0,100,0),1);
    baseNode::wifiConnected();
  }

};

#endif
