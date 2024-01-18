#ifndef LED_MATRIX_H
#define LED_MATRIX_H


#include "baseNode.h"
#include "ledGadget.h"

class ledMatrixNode : public baseNode
{
public:
  ledMatrixNode(storage* s) : baseNode(s),
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
  ledMatrix _leds;

};




#endif
