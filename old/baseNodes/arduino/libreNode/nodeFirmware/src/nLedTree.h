#ifndef CTREE


#include "nLedBar.h"
#include "nodeComponent.h"

class ledTreeNode : public ledBarNode
{
public:
  ledTreeNode(storage* s) : ledBarNode(s),
  _bodyLeds   (String(F("bodyLeds")),s,&_ledController,0,580,true),
  _starLeds (String(F("starLeds")),s,&_ledController,580,20)
  {
    addComponent(&_bodyLeds);
    addComponent(&_starLeds);
  }

  void setupNode()
  {
    christmasLights();
  }

  void nodeLoop()
  {

  }

  void christmasLights()
  {
    _bodyLeds.christmas();
    //_starLeds.setColor(200,200,0);
  }
protected:
  ledBar     _bodyLeds;
  ledBar     _starLeds;
};

#endif
