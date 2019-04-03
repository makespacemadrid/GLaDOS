#ifndef RELAY_H
#define RELAY_H

#include "gladosMQTTNode.h"

class relay : public nodeComponent
{
  public:
    relay(int pin = -1)
    {
		m_pin = pin;
    }

    bool canRead()
    {
        return true;
    }

    void setInvertedLogic(bool inverted)
    {
        m_invertedLogic = inverted;
    }

    uint16_t readRaw()
    {
        return m_status;
    }

    float readData()
    {
        return m_status;
    }

    virtual void activate()
    {
        if(!m_status)
          setStatus(true);
    }

    virtual void deactivate()
    {
        if(m_status)
          setStatus(false);
    }

    virtual void toggle()
    {
      setStatus(!m_status);
    }
    
    bool getStatus()
    {
        return m_status;
    }

    void setStatus(bool status)
    {
        if(!isValid()) return;
        m_status = status;

        if(m_invertedLogic)
            digitalWrite(m_pin,!m_status);
        else
            digitalWrite(m_pin,m_status);

    }

    virtual bool isValid()
    {
        return m_pin != -1;
    }

    virtual void setup()
    {
        if(!isValid()) return;
        pinMode(m_pin, OUTPUT);
    }

  protected:
	int  m_pin;
    bool m_status = false;
    bool m_invertedLogic = false;
};


#endif // RELAY_H
