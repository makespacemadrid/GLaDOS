#ifndef EEPROMSTORAGE
#define EEPROMSTORAGE


#include "nodeclient.h"

class EEPROMStorage : public settingsStorage
{
public:
  EEPROMStorage() : settingsStorage()
  {
      load();
      Serial.println("EEPROM ready!");
  }

 ~EEPROMStorage() {;}

  bool hasNodeSettings()
  {
      nodeSettings settings;
      settings.magicNumber = 0;
      EEPROM.begin(m_flashSize);
      EEPROM.get(0,settings);
      EEPROM.end();
      return settings.magicNumber == nodeSettings().magicNumber;
  }


  void clear()
  {
      EEPROM.begin(m_flashSize);
      for ( int i = 0 ; i < m_flashSize ; i++ )
        EEPROM.write(i, 0);
      EEPROM.end();
  }


protected:
  uint16_t    m_flashSize   = 1024;

  bool loadNodeSettings()
  {
      nodeSettings settings;
      settings.magicNumber = 0;
      EEPROM.begin(m_flashSize);
      EEPROM.get(0,settings);
      EEPROM.end();

      if(settings.magicNumber == nodeSettings().magicNumber)
      {
          m_nodeSettings = settings;
          return true;
      }
      else
          return false;
  }


  void saveNodeSettings()
  {
      EEPROM.begin(m_flashSize);
      Serial.print("Saving Basic Settings..");
      EEPROM.put(0,m_nodeSettings);
      EEPROM.commit();
      EEPROM.end();
  }

};

#endif // EEPROMSTORAGE
