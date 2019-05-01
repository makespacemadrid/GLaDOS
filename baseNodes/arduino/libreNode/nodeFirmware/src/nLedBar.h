#ifndef LEDBAR
#define LEDBAR

#include "baseNode.h"
#include "ledGadget.h"

class ledBarNode : public baseNode
{
public:
  ledBarNode(storage* s) : baseNode(s),
  _leds("led",s,&_ledController)
  {
    addComponent(&_leds);
    addledGadget(&_leds);
  }

  virtual void setupNode()  {;}
  virtual void sensorLoop() {;}
  virtual void nodeLoop()   {;}

private:

protected:
  ledBar _leds;

};

#endif
