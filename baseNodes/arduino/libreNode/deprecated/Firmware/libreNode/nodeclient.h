#ifndef NODECLIENT_H
#define NODECLIENT_H


#define P_PACKETSTART "[[[[|||"
#define P_PACKETEND   "|||]]]]"

#define MAXSTRLENGTH 20
#define ML MAXSTRLENGTH

#include "qtcompat.h"
#include "vector"

enum nodeWifiMode
{
    wifiAP,
    wifiClient,
    wifiMesh,
    wifiOverride
};

struct nodeSettings
{
    char id[20]                     = "nodeClient";

    //Watchdog
    uint16_t watchdogTime           = 5000;
    uint16_t delayTime              = 5;
    uint16_t highFreqCycleMs        = 1;
    uint16_t medFreqCycleMs         = 30;
    uint16_t lowFreqCycleMs         = 250;


    //Wireless
    nodeWifiMode    wifiMode               =  wifiAP;
    char            wifiESSID         [ML] = "Configureme";
    char            wifiPasswd        [ML] = "configureme";
    char            remoteHost        [ML] = "server.local";
    uint16_t        remotePort             =  31416;

    char     wifiOverrideESSID [ML] = "AndroidAP"; //Si este ssid esta presente en el arranque se conectara con el.
    char     wifiOverridePasswd[ML] = "clubmate";
    char     overrideRemoteHost[ML] = "192.168.43.1";
    uint16_t overrideRemotePort     =  31416;

    uint16_t localPort              = 31416;
    uint16_t httpUpdatePort         = 8080;
    uint16_t httpPort               = 80;
    uint8_t  IP[4]                  = {2,44,97,150};
    uint8_t  subnet[4]              = {255,255,0,0};

    bool     staticIP                  = false;
    bool     allowWifiOverride         = true;
    bool     autoConnectRemote         = false;
    bool     overrideAutoConnectRemote = true;
    bool     localPortEnabled          = true;
    bool     httpUpdaterEnabled        = true;
    bool     httpServerEnabled         = true;
    bool     MdnsEnabled               = true;
    bool     serialClient              = false;

    uint16_t magicNumber               = 31415; //numero de comprobacion para saber que los settings se han leido/transmitido bien.
};

enum nodeProtocolMessages
{
    cmdDummy,
    cmdGetSettings,
    cmdReloadSettings,
    cmdSaveSettings,
    cmdFactorySettings,
    cmdReset,
    cmdPing,
    respPong,
    cmdVersion,
    respVersion
};

struct nodeProtocolPacket
{
    uint16_t    msgID       = cmdDummy;
    uint16_t    arg0        = 0;
    uint16_t    magicNumber = 31416;
};


static void copyRawBytes(uint8_t* source, uint8_t* dest,uint16_t size)
{
    for(uint16_t i = 0 ; i < size ; i++)
        dest[i] = source[i];
}

static void copyRawBytes(String& source, uint8_t* dest,uint16_t size)
{
    for(uint16_t i = 0 ; i < size ; i++)
        dest[i] = (uint8_t)source.charAt(i);
}

static void copyRawBytes(String& source, nodeProtocolPacket& dest)
{
    uint8_t* byteStorage = (uint8_t*)&dest;
    for(uint16_t i = 0 ; i < sizeof(nodeProtocolPacket) ; i++)
        byteStorage[i] = (uint8_t)source.charAt(i);
}

static std::vector<String> splitStr(String& str,String sep)
{
        std::vector<String> result;
  if(str.length() < 1) return result;
  int sep_index = str.indexOf(sep);
  if(sep_index == -1)
  {//no hay separador, se devuelve un unico elemento
    result.push_back(str);
    return result;
  }

  while (sep_index >= 0)
  {
    result.push_back(str.substring(0,sep_index));
    str = str.substring(sep_index+sep.length());
    sep_index = str.indexOf(sep);
  }
  if(str.length()) result.push_back(str);
        return result;
}

static int indexOfStringInString(String& str, String value)
{
    uint8_t valSize = value.length();
    char    val[valSize+1];
    value.toCharArray(val,valSize+1);
    //Serial.print("Searching val: ");Serial.print(value);Serial.print(" strLen:");Serial.println(str.length());
    if(str.length() < valSize)
        return -1;

    for(int p = 0 ; p < str.length()-valSize+1 ; p++)
    {
        char    data[valSize+1];
        int i = 0;
        bool found = true;
        for(int c = p ; c < p+valSize ; c++)
        {
            data[i] = str.charAt(c);
            //Serial.print("Char : ");Serial.print(c);Serial.print("  Val: "); Serial.print((uint8_t)data[i]); Serial.print("\tExpected:");Serial.println((uint8_t)val[i]);
            if(val[i] != data[i])
            {
                found = false;
                break;
            }
            i++;
        }
        if(found)
            return p;
    }
    return -1;
}


class settingsStorage
{
public:
    settingsStorage()
    {

    }

   ~settingsStorage() {;}

    virtual void save()
    {
        saveNodeSettings();
    }

    virtual void load()
    {
        loadNodeSettings();
    }

    nodeSettings&  getBasicSettings()
    {
        return m_nodeSettings;
    }

    void getNodeSettingsBytes(uint8_t* result)
    {
        copyRawBytes((uint8_t*)&m_nodeSettings,result,sizeof(nodeSettings));
    }

    void setNodeSettingsBytes(uint8_t* byteStorage)
    {
        nodeSettings result;
        result.magicNumber = 0;
        copyRawBytes(byteStorage,(uint8_t*)&result,sizeof(nodeSettings));
        if(result.magicNumber == nodeSettings().magicNumber)
            m_nodeSettings = result;
    }

    void setNodeSettingsBytes(String& str)
    {
        nodeSettings result;
        result.magicNumber = 0;
        copyRawBytes(str,(uint8_t*)&result,sizeof(nodeSettings));
        if(result.magicNumber == nodeSettings().magicNumber)
        {
            m_nodeSettings = result;
        }
    }

    virtual bool            hasNodeSettings(){return false;}
    virtual void            clear(){;}

protected:
    nodeSettings   m_nodeSettings;

    virtual bool loadNodeSettings() {return false;}
    virtual void saveNodeSettings() {;}
};


class commModule
{
public:
    commModule(nodeSettings* s) : m_nodeSettings(s)
    {

    }

    virtual void    setup();
    virtual bool    isOpen();
    virtual String  read();
    virtual void    write(char* data,uint16_t size);

    String& buffer() {return m_buffer;}

protected:
    String  m_buffer;
    nodeSettings* m_nodeSettings;
};


class nodeClient
{
public:
    nodeClient()
    {

    }

    void sendNodeProtocolPacket(nodeProtocolMessages msgid, uint16_t arg = 0)
    {
        nodeProtocolPacket msg;
        msg.msgID = msgid;
        msg.arg0  = arg;
        sendSimpleMsg(msg);
    }

    void sendSimpleMsg(nodeProtocolPacket& msg)
    {
        char result[sizeof(nodeProtocolPacket)];
        copyRawBytes((uint8_t *)&msg,(uint8_t*)result,sizeof(nodeProtocolPacket));
        encodeAndSend(result,sizeof(nodeProtocolPacket));
    }

    virtual void sendSettings()
    {
        sendNodeSettings();
    }

    void encodeAndSend(char* msg, uint16_t count)
    {
        String pStart;
        pStart += P_PACKETSTART;
        String pEnd;
        pEnd   += P_PACKETEND;
        uint16_t totalBytes = pStart.length() + count + pEnd.length();
        char result[pStart.length()+pEnd.length()+count+1];

        for(int i = 0 ; i < pStart.length() ; i++)
            result[i] = pStart.charAt(i);

        int c = 0;
        for(int i = pStart.length() ; i < count+pStart.length() ; i++)
            result[i] = msg[c++];

        c = 0;
        for(int i = pStart.length() + count ; i < totalBytes  ; i++)
            result[i] = pEnd.charAt(c++);
        result[totalBytes+1] = 0;
        m_commModule->write(result,totalBytes);

    }

    void read()
    {

    }


protected:
    commModule*         m_commModule;
    settingsStorage*    m_settingsStorage;

    void parseBuffer(String& buf)
    {
        //Cosas raras pasan con los String cargados de caracteres binarios, hay que volcarlos a mano como bytes,
        // No se puede usar ni el igual ni el subString en arduino!!
        if(buf.length() == 0) return;
        String pStart;
        pStart += P_PACKETSTART;
        String pEnd;
        pEnd   += P_PACKETEND;
        int s_index =  indexOfStringInString(buf,pStart);
        int e_index =  indexOfStringInString(buf,pEnd);

        //saneado del buffer
        if(s_index < 0)
        {// si no hay inicio de paquete lo que hay en el buffer tiene que ser basura.
            //Serial.print("Garbage on input buffer, cleaning ");Serial.print(buf.length());Serial.println("bytes");
            buf = String();
            return;
        }
        int s_lenght = pStart.length();
        int e_lenght = pEnd.length();
        //extraccion de comandos
        while ((s_index >= 0) && (e_index > (s_index+s_lenght))) //Si hay inicio y fin de paquete se extrae el comando.
        {// lo que haya en el buffer hasta el inicio de paquete se descarta(basura)
            String packet;
            for(int i = s_index+s_lenght ; i < e_index ; i++)
            {
                packet += buf.charAt(i);
            }
//            Serial.print("Buffer Had: "); Serial.print(buf.length());Serial.print("bytes, getting from:");Serial.print(s_index+s_lenght);Serial.print(" to: ");Serial.println(e_index);
            String tmp;
            for(int i = e_index+e_lenght ; i < buf.length(); i++ )
                tmp += buf.charAt(i);
            buf = String();
            for(int i = 0 ; i < tmp.length(); i++ )
                buf += tmp.charAt(i);
            parsePacket(packet);
            s_index = indexOfStringInString(buf,pStart);
            e_index = indexOfStringInString(buf,pEnd);

            //Serial.print(buf.length()); Serial.println("bytes left on rx buffer\n\n");
            yield();
        }
    }

    void parsePacket(String& str)
    {

        //Serial.print("ParsePacket:"); Serial.print(str.length()); Serial.println("Bytes");
        if      (str.length() == sizeof(nodeProtocolPacket))
            parseNodeProtocolPacket(str);
        else if (str.length() == sizeof(nodeSettings))
            m_settingsStorage->setNodeSettingsBytes(str);
        else
            parsePacketExtra(str);
    }

    virtual void parsePacketExtra(String&) {;}

    void parseNodeProtocolPacket(String& str)
    {
        nodeProtocolPacket msg;
        msg.magicNumber = 0;
        copyRawBytes(str,msg);
        if(msg.magicNumber == nodeProtocolPacket().magicNumber)
        {
            //Serial.println("... OK!");
        }
        else
        {
            //Serial.print("...  FAIL! magic number ="); Serial.println(msg.magicNumber);
            return;
        }
        //Serial.print("Procesando Mensaje : ");Serial.println(msg.msgID);

        if(msg.msgID == cmdGetSettings)
        {
            sendSettings();
        }
        else if(msg.msgID == cmdPing)
        {
            sendNodeProtocolPacket(respPong);
        }
        else if(msg.msgID == cmdSaveSettings)
        {
            m_settingsStorage->save();
            sendSettings();
        }
        else if(msg.msgID == cmdReloadSettings)
        {
            m_settingsStorage->load();
            sendSettings();
        }
        else if(msg.msgID == cmdFactorySettings)
        {
            m_settingsStorage->clear();
            m_settingsStorage->load();
            sendSettings();
        }
        else if(msg.msgID == cmdReset)
        {
            //
        }
    }

    void sendNodeSettings()
    {
        char data[sizeof(nodeSettings)];
        m_settingsStorage->getNodeSettingsBytes((uint8_t*)data);
        encodeAndSend(data,sizeof(nodeSettings));
    }

};

#endif // NODECLIENT_H
