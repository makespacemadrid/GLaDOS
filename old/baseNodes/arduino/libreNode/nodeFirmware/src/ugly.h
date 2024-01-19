#ifndef UGLY_H
#define UGLY_H

void startOTA(String hostName)
{
  //OTA
    ArduinoOTA.setHostname(hostName.c_str());
    //ArduinoOTA.setPassword("");
    ArduinoOTA.onStart    ([](){ledsOTAMode();});
    ArduinoOTA.onEnd      ([](){ledsOTAEnd();Serial.println("End!");});
    ArduinoOTA.onProgress ([](unsigned int progress, unsigned int total) {ledsOtaProgress(progress*100/total);});
    ArduinoOTA.onError    ([](ota_error_t error)                         {ledsOTAError();ESP.restart();});
    ArduinoOTA.begin();
  //
}

void ledsOTAMode()
{
  node->lController()->setColor(50,50,0);
  node->lController()->show();
}
void ledsOTAEnd()
{
  node->lController()->setColor(0,50,0);
  node->lController()->show();
}
void ledsOtaProgress(uint8_t p)
{
  node->lController()->progress(p,CRGB(60,60,0));
}
void ledsOTAError()
{
  node->lController()->setColor(50,0,0);
  node->lController()->show();
}
void ledsOff()
{
  node->lController()->off();
}

void mqttCallback(char* topic, byte* payload, unsigned int length)
{
	node->readTopic(topic,payload,length);
}

bool enterConfigMode(storage* s)
{
  if(s->getNodeConfig(FIRST_RUN) == "Y")
  {
    s->setNodeConfig(FIRST_RUN,"N");
    return true;
  }
  if(s->getNodeConfig(NODE_TYPE) == NODE_NEW)
  {
    return true;
  }

  String cMode = s->getNodeConfig(CONFIG_MODE);
  if(cMode == ON_BOOT_ERROR)
  {
    int bootError = String(s->getKeyValue(BERROR,BOOT_CONFIG)).toInt();
    bootError++;
    if(bootError >= String(s->getNodeConfig(MAX_BERROR)).toInt())
    {
      s->setKeyValue(BERROR,String(0),BOOT_CONFIG);
      return true;
    }
    else
      s->setKeyValue(BERROR,String(bootError),BOOT_CONFIG);
  }
  else if(cMode == ON_PIN)
  {
    uint8_t pin = String(s->getNodeConfig(CONFIG_PIN)).toInt();
    pinMode(pin,INPUT);
    delay(50);
    if(digitalRead(pin) == HIGH) return true;
  }
  return false;
}
#endif
