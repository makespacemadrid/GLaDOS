#ifndef BUTTONS_H
#define BUTTONS_H

#include "gladosMQTTNode.h"

class button : public nodeComponent
{
public:
  button(int pin = -1) : nodeComponent(),  m_invertedLogic(false), m_statusChanged(false)
  {
	m_pin = pin;
	m_id  = "button";
  }

  bool     canRead()  {return true;}

  virtual uint16_t readRaw()
  {
      if(m_invertedLogic)
        return !digitalRead(m_pin);
      else
        return digitalRead(m_pin);
  }

  virtual float     readData()
  {
      return readRaw();
  }

  virtual void setup()
  {
      pinMode(m_pin, INPUT);
      delay(5);
      bool state = readRaw();
      m_lastStatus = state;
  }

  bool statusChanged()
  {
    if(m_statusChanged)
    {
      m_statusChanged = false;
      return true;
    }
    return false;
  }

  bool status()
  {
      return m_lastStatus;
  }


  virtual void updateComponent()
  {//añadir mecanismo de debounce?
      bool status = readRaw();
      if(status != m_lastStatus)
      {
          m_lastStatus = status;
          m_statusChanged = true;
      }
  }

  void setInvertedLogic(bool inverted) {m_invertedLogic = inverted;}

  void readComponent()
  {
	mqttPublication p;
	p.path  = m_topicPath;
	p.topic = m_id;
	p.val	= String(readData());
	Serial.print("readComponent()->");Serial.println(p.getTopic());
	m_publications.push_back(p);
  }

protected:
  int  m_pin;
  bool m_invertedLogic;
  bool m_lastStatus;
  bool m_statusChanged;

};


class potenciometer : public nodeComponent
{
public:
    potenciometer(int pin = -1) : nodeComponent()
    {
      m_pin = pin;
    }

    bool canRead()
    {
        return true;
    }

    uint16_t readRaw()
    {
        return analogRead(m_pin);
    }

    float readData()
    {
        uint8_t samples = 5;
        float value = readRaw();
        for(int i = 1 ; i < samples ; samples++)
        {
            value += readRaw();
            value /= 2;
        }
        return value/10.f;
    }

protected:
	int m_pin;
};

#endif // BUTTONS_H
