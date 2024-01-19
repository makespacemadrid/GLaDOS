#ifndef COMMESP_H
#define COMMESP_H

//#ifdef ESP8266

#include "nodeclient.h"
#include "esphelpers.h"


class modEsp8266 : public commModule
{
public:
    modEsp8266(nodeSettings* s) : commModule(s)
    {

    }
    virtual void    setup()
    {
        esp_init(m_nodeSettings);
    }

    virtual bool    isOpen();
    virtual String  read();
    virtual void    write(char* data,uint16_t size);
protected:
    WiFiServer                   TCPserver;

};


//#endif
#endif // COMMESP_H
