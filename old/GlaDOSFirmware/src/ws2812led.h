#ifndef LED
#define LED


#include <Adafruit_NeoPixel.h> //WTF hay que hacer el include en el ino!
#include <vector>

#include "gladosMQTTNode.h"


class ws2812Strip : public nodeComponent {
public:
    class led{ // clase led dentro de la clase Strip
    public:
      led(int r = 0, int g = 0, int b = 0, float bright = 1.0): m_r(r), m_g(r), m_b(b), m_bright(bright) {;}

      void setColor(uint8_t red =0,uint8_t green=0,uint8_t blue=0)
      {
          m_r = red;
          m_g = green;
          m_b = blue;
      }

      void dimm(int power)
      {
          float p = power /100;
          dimm(p);
      }

      void dimm(float factor = 0.75)
      {
          m_r *= factor;
          m_g *= factor;
          m_b *= factor;
      }

      void setBrightness(float b = 1.0)
      {
          m_bright = b;
      }

      void off()   {setColor(0,0,0);}
      void white() {setColor(200,200,200);}
      void red()   {setColor(230,0,0);}
      void green() {setColor(0,230,0);}
      void blue()  {setColor(0,0,230);}

      void setR(uint8_t val) {m_r = val;}
      void setG(uint8_t val) {m_g = val;}
      void setB(uint8_t val) {m_b = val;}

      int r()   {return m_r * m_bright;}
      int g()   {return m_g * m_bright;}
      int b()   {return m_b * m_bright;}

    private:
      uint8_t m_r;
      uint8_t m_g;
      uint8_t m_b;
      float   m_bright;
    };

  ws2812Strip(int pin = -1, int lednr = 25, float bright = 1.0) : m_pin(pin) ,nodeComponent(),  m_pixels(lednr, pin, NEO_GRB + NEO_KHZ800)
  {
    m_brightness = bright;
    if((isValid()))
    {
      for(int i = 0 ; i < lednr ; i++) m_leds.push_back(led());
    }
  }

  void setBrightness(int b)
  {
      m_brightness = float(b)/100.0f;
      update();
  }

  void setBrightness(float b = 1.0)
  {
      m_brightness = b;
      update();
  }

  float brightness()
  {
      return m_brightness;
  }

  bool isValid() {return  (m_pin != -1);}

  std::vector<led> leds()&   {return m_leds;}

  led* getLed(int pos)
  {
    return &m_leds[pos];
  }

  void setup()
  {
    Serial.println("Setting up ws2812 strip....");
    m_pixels.begin();
    yield();
    test();
    off();
  }

  void updateComponent(){
    for(int i=0;i<m_leds.size();i++)
    {
        m_pixels.setPixelColor(i, (m_leds[i].r() * m_brightness) ,
                                  (m_leds[i].g() * m_brightness) ,
                                  (m_leds[i].b() * m_brightness) );
    }

    yield();
    m_pixels.show();
  }

  void off()
  {
    for( uint16_t i = 0 ; i < m_leds.size() ; i++)
    {
        m_leds[i].off();
    }
    update();
  }

  void setColor(uint8_t r,uint8_t g, uint8_t b){
    if(!isValid()) return;
    for(int i=0;i<m_leds.size();i++)
    {
      m_leds[i].setColor(r,g,b);
    }
    update();
  }

  void test()
  {
    for( uint16_t i = 0 ; i < m_leds.size() ; i++)
    {
      m_leds[i].setColor(20,20,20);
      if(i%2 == 0)update();
    }
  }

private:
    std::vector<led>    m_leds;
    int                 m_pin;
    float               m_brightness;
    Adafruit_NeoPixel   m_pixels;
};


class ledGadget : public nodeComponent
{
  public:
    enum animationType
    {
        animationNone,
        animationColor,
        animationFade,
        animationGlow,
        animationBlink,
        animationCylon,
        animationChaoticLight,
        animationRainbow,
        animationScroll
    };

    ledGadget(ws2812Strip* m_parentStrip): nodeComponent() , m_strip(m_parentStrip) , m_bright(1.0)
    {

    }

    void updateComponent()
    {
      animate();
    }

    bool isValid() {return m_strip->isValid();}

    void deactivate()
    {
        fade();
    }

//gestion del array de leds

    void setLeds(std::vector<String>&  args)
    {
      m_animationType = animationNone;
      for(int i = 0 ; i < args.size(); i+=3)
      {
          int n = i/3;
          if( n <= m_leds.size())
          {
            ws2812Strip::led* l = m_leds[n];
            l->setColor(args[i].toInt(),args[i+1].toInt(),args[i+2].toInt());
          }
      }
    }

    void setLeds(std::vector<uint8_t>  args)
    {
      m_animationType = animationNone;
      for(int i = 0 ; i < args.size(); i+=3)
      {
          int n = i/3;
          if( n <= m_leds.size())
          {
            uint8_t r = args[i];
            uint8_t g = args[i+1];
            uint8_t b = args[i+2];
            ws2812Strip::led* l = m_leds[n];
            l->setColor(r,g,b);
          }
      }
    }

    void addLed(ws2812Strip::led* nLed)
    {
        m_leds.push_back(nLed);
    }

    void addLed(int start,int count,bool reverseOrder = false)
    {
        if(reverseOrder)
        {
            for(int i = start+count-1 ; i >= start ; i-- )
            {
                m_leds.push_back(m_strip->getLed(i));
            }
        }
        else
        {
            for(int i = start ; i < start+count ; i++ )
            {
                m_leds.push_back(m_strip->getLed(i));
            }
        }
    }

//efectos y animaciones

    virtual void off()
    {
        resetAnimation();
        for(int i = 0 ; i < m_leds.size() ; i++)
            m_leds[i]->off();
        m_strip->update();
    }

    virtual void setColor(uint8_t r, uint8_t g, uint8_t b)
    {
        resetAnimation();
        for(int i = 0 ; i < m_leds.size() ; i++)
        {
            m_leds[i]->setColor(r,g,b);
        }
    }

    void dimm(uint8_t power)
    {
        if(power > 100) power = 100;
        setBrightness(power /100.0f);
    }

    void setBrightness(float b = 1.0)
    {
        m_bright = b;
        for(int i = 0 ; i < m_leds.size() ; i++)
        {
            m_leds[i]->setBrightness(b);
        }
        m_strip->update();
    }


    virtual void setBaseColor(uint8_t r, uint8_t g, uint8_t b)
    {
      m_baseColoR = r;
      m_baseColoG = g;
      m_baseColoB = b;
    }

    uint8_t baseColorR()
    {
      return m_baseColoR;
    }

    uint8_t baseColorG()
    {
      return m_baseColoG;
    }

    uint8_t baseColorB()
    {
      return m_baseColoB;
    }

    virtual void resetAnimation()
    {
        if(m_animationType == animationGlow)
        {
            setBrightness(1.0);
        }

        m_animationType = animationNone;
        m_counter1 = 0, m_counter2 = 0 ; m_counter3 = 0;
    }

    virtual void baseColor()
    {
      setColor(m_baseColoR, m_baseColoG,m_baseColoB);
      resetAnimation();
      m_animationType = animationColor;
    }

    virtual void fade()
    {
        resetAnimation();
        m_animationType = animationFade;
    }

    virtual void instantFade()
    {
      fade();
      while (m_animationType != animationNone)
      {
        animateFade();
        m_strip->update();
        delay(10);
      }
    }

    virtual void instantBlink(uint8_t r, uint8_t g, uint8_t b, uint times = 2,uint length = 250)
    {
      instantFlash(r, g,  b, times,length);
    }

    virtual void instantFlash(uint8_t r, uint8_t g, uint8_t b, uint times = 2,uint length = 250)
    {
      animationType last = m_animationType;
      for(int t = 0 ; t<times ; t++)
      {
        setColor(r,g,b);
        animate();
        delay(length);
        instantFade();
      }
      m_animationType = last;
    }

    void randomColor()
    {
      setColor(rand()%255,rand()%255,rand()%255);
    }

    virtual void glow()
    {
        resetAnimation();
        m_counter1 = 200, m_counter2 = 1 ; m_counter3 = 200;
        m_animationType = animationGlow;
    }

    virtual void blink()
    {
        resetAnimation();
        m_counter1 = 0, m_counter2 = 0 ; m_counter3 = 0;
        m_animationType = animationBlink;
    }

    virtual void cylon()
    {//animacion cylon
        resetAnimation();
        m_counter1 = 0, m_counter2 = 0 ; m_counter3 = 0;
        m_animationType = animationCylon;
    }

    virtual void chaoticLight()
    {
        resetAnimation();
        m_counter1 = 0, m_counter2 = 0 ; m_counter3 = 0;
        m_animationType = animationChaoticLight;
    }

    virtual void rainbow()
    {
        resetAnimation();
        m_counter1 = 0, m_counter2 = 0 ; m_counter3 = 0;
        m_animationType = animationRainbow;
    }

    virtual void scroll()
    {
        resetAnimation();
        m_counter1 = 0, m_counter2 = 0 ; m_counter3 = 0;
        m_animationType = animationScroll;
    }

    virtual void setProgress(uint8_t progress, uint8_t r = 0,uint8_t g = 100,uint8_t b = 0, bool centered = false)
    {
      if(progress >100)
        progress = 100;
      if(centered)
      {
        setColor(0,0,0);
        int lit = m_leds.size()*(progress/100.0);
        for(int l = 0 ; (l < lit) && (l < m_leds.size()) ; l++)
          m_leds[l]->setColor(r,g,b);
      }
      else
      {
        setColor(0,0,0);
        int lit = m_leds.size()*(progress/100.0);
        for(int l = 0 ; (l < lit) && (l < m_leds.size()) ; l++)
          m_leds[l]->setColor(r,g,b);
      }
    }


    void setAnimationJSON(String json)
    {
      StaticJsonBuffer<200>  data;
      JsonObject& c = data.parseObject(json);
      if (!c.success())
      {
        Serial.println("Parsing json failed !");
        return;
      }
      String t = c["animation"];

      if(t == "Color")
      {
        String sr = c["r"]; int r = sr.toInt();
        String sg = c["g"]; int g = sg.toInt();
        String sb = c["b"]; int b = sb.toInt();
        setColor(r,g,b);
      }else if(t == "Fade")
      {
        instantFade();
      }else if(t == "Glow")
      {
        glow();
      }else if(t == "Flash")
      {
        String sr = c["r"]; int r = sr.toInt();
        String sg = c["g"]; int g = sg.toInt();
        String sb = c["b"]; int b = sb.toInt();
        String st = c["times"]; int times = st.toInt();
        String sl = c["length"]; int l = sl.toInt();

        instantFlash(r, g, b,times,l);
      }else if(t == "Cylon")
      {
          cylon();
      }else if(t == "ChaoticLight")
      {
          chaoticLight();
      }else if(t == "Rainbow")
      {
          rainbow();
      }else if(t == "Progress")
      {
          String sr = c["val"]; int p = sr.toInt();
          setProgress(p);
      }
    }

    virtual void animate()
    {
        if      (m_animationType == animationNone)
        {
            return;
        }else if(m_animationType == animationFade)
        {
            animateFade();
        }else if(m_animationType == animationColor)
        {
            baseColor();
        }else if(m_animationType == animationGlow)
        {
            animateGlow();
        }else if(m_animationType == animationBlink)
        {
            animateBlink();
        }else if(m_animationType == animationCylon)
        {
            animateCylon();
        }else if(m_animationType == animationChaoticLight)
        {
            animateChaoticLigth();
        }else if(m_animationType == animationRainbow)
        {
            animateRainbow();
        }else if(m_animationType == animationScroll)
        {
            animateScroll();
        }

        m_strip->update();
    }

    void wheel(int pos)
    {
      resetAnimation();
      for(uint16_t i = 0;  i < m_leds.size() ; i++)
      {
          uint16_t WheelPos = 255 - (((i * 256 / m_leds.size()) + pos) & 255);
          if(WheelPos < 85)
          {
              m_leds[i]->setColor(255 - WheelPos * 3, 0, WheelPos * 3);
          }
          else if(WheelPos < 170)
          {
              WheelPos -= 85;
              m_leds[i]->setColor(0, WheelPos * 3, 255 - WheelPos * 3);
          }
          else
          {
              WheelPos -= 170;
              m_leds[i]->setColor(WheelPos * 3, 255 - WheelPos * 3, 0);
          }
    }
  }


  protected:
    ws2812Strip*                    m_strip;
    std::vector<ws2812Strip::led*>  m_leds;
    animationType                   m_animationType = animationNone;
    float m_bright;

    uint8_t m_counter1;
    uint8_t m_counter2;
    uint8_t m_counter3;

    uint8_t         m_lastR;
    uint8_t         m_lastG;
    uint8_t         m_lastB;

    uint8_t         m_baseColoR;
    uint8_t         m_baseColoG;
    uint8_t         m_baseColoB;


    virtual void animateFade()
    {
        bool all_off = true;
        for(int i = 0 ; i < m_leds.size() ; i++)
        {
            if( (m_leds[i]->r() > 0) || (m_leds[i]->g() > 0) || (m_leds[i]->b() > 0) )
            {
                m_leds[i]->dimm();
                all_off=false;
            }
        }

        if(all_off)
        {
            resetAnimation();
        }
    }

    virtual void animateGlow()
    {//contador 1 se usa contar la iteracion, y el 2 para la direccion(sumando/restando).
        if(m_counter2 == 0)
        {//sumando
            float bright = m_counter1/200.0;
            for(int i = 0 ; i < m_leds.size() ; i++)
            {
                m_leds[i]->setBrightness(bright);
            }

            if(m_counter1 == 200)
              m_counter2 = 1;
            else
              m_counter1++;
        }
        else
        {//restando
            float bright = m_counter1/200.0;
            for(int i = 0 ; i < m_leds.size() ; i++)
            {
                m_leds[i]->setBrightness(bright);
            }

            if(m_counter1 == 0)
                m_counter2 = 0;
            else
              m_counter1--;
        }
 //       Serial.print("C1: ");
 //       Serial.print(m_counter1);
 //       Serial.print(" :C2: ");
 //       Serial.print(m_counter2);
 //       Serial.print(" :C3: ");
 //       Serial.println(m_counter3);
 //       Serial.println("   ");
 //       Serial.println(m_leds[0]->b);
    }

    virtual void animateBlink()
    {

    }

    virtual void animateCylon()
    {//contador 1 para la iteracion, 2 para la direccion
        int min = 0;
        int max = m_leds.size();
        int count = max - min;

        if(m_counter2 == 0)//incrementando
        {
            int i = 0;
            for(int l = min ; l < max ; l++ )
            {
                ws2812Strip::led *led = m_leds[l];
                if(i == m_counter1)
                {
                   led->setColor(255,0,0);
                }
                else
                {
                    led->dimm();
                }

                if(m_counter1 >= max)
                {
                    m_counter1 = max;
                    m_counter2 = 1;
                    return;
                }
                else
                {
                    i++;
                }
            }
            m_counter1++;
        }else //decrementando
        {
            int i = count;
            for(int l = max-1 ; l >= min ; l-- )
            {
                ws2812Strip::led *led = m_leds[l];
                if(i == m_counter1)
                {
                   led->setColor(255,0,0);
                }
                else
                {
                    led->dimm();
                }

                if(m_counter1 <= min)
                {
                    m_counter1 = 0;
                    m_counter2 = 0;
                    return;
                }
                else
                {
                    i--;
                }
            }
            m_counter1--;
        }
    }
    virtual void animateChaoticLigth()
    {
        for(int i = 0 ; i < m_leds.size() ; i++)
        {
            int random = rand() % 3;
            if(random == 0)
                m_leds[i]->red();
            else if(random == 1)
                m_leds[i]->green();
            else if(random == 2)
                m_leds[i]->blue();
        }
    }

    virtual void animateRainbow()
    {
        if(m_counter1 > 256*5) m_counter1 = 0;

        for(uint16_t i = 0;  i < m_leds.size() ; i++)
        {
            uint16_t WheelPos = 255 - (((i * 256 / m_leds.size()) + m_counter1) & 255);
            if(WheelPos < 85)
            {
                m_leds[i]->setColor(255 - WheelPos * 3, 0, WheelPos * 3);
            }
            else if(WheelPos < 170)
            {
                WheelPos -= 85;
                m_leds[i]->setColor(0, WheelPos * 3, 255 - WheelPos * 3);
            }
            else
            {
                WheelPos -= 170;
                m_leds[i]->setColor(WheelPos * 3, 255 - WheelPos * 3, 0);
            }
            yield();
        }
        m_counter1 += 1;
    }

    virtual void animateScroll() {;}
};

class ledBar : public ledGadget
{
public:
    ledBar(ws2812Strip* parentStrip) : ledGadget(parentStrip), m_reversed(false)
    {
    }
    void setReversed(bool reversed) {m_reversed = reversed;}

protected:
    bool m_reversed;

};


class ledStatusTrio : public ledBar
{
  public:
    ledStatusTrio(ws2812Strip* m_parentStrip, uint8_t mControllerLed = 0, uint8_t wifiLed = 1, uint8_t extraLed= 2) :
       ledBar(m_parentStrip)
    {
        m_controllerLed = m_strip->getLed(mControllerLed);
        m_wifiLed       = m_strip->getLed(wifiLed);
        m_extraLed      = m_strip->getLed(extraLed);
        addLed(m_controllerLed);
        addLed(m_wifiLed);
        addLed(m_extraLed);
    }

    void animate()
    {
        ledBar::animate();

        if(m_statusDecay)
            m_controllerLed->dimm();
        if(m_wifiStatusDecay)
            m_wifiLed->dimm();
        if(m_extraLedDecay)
            m_extraLed->dimm();
        m_strip->update();
    }

    void controllerInit()
    {
        m_statusDecay = false;
        m_controllerLed->setColor(0,100,100);
    }

    void controllerOK()
    {
        m_statusDecay = true;
        m_controllerLed->setColor(0,100,0);
    }

    void controllerError()
    {
        m_statusDecay = false;
        m_controllerLed->setColor(100,0,0);
    }

    void wifiTX()
    {
        m_wifiStatusDecay = true;
        m_wifiLed->setColor(0,100,0);
    }

    void wifiRX()
    {
        m_wifiStatusDecay = true;
        m_wifiLed->setColor(100,0,0);
    }

    void wifiInit()
    {
        m_wifiStatusDecay = false;
        m_wifiLed->setColor(100,100,0);
    }

    void wifiClient()
    {
        m_wifiStatusDecay = false;
        m_wifiLed->setColor(0,100,100);
        m_wifiStatusColor = *m_wifiLed;
    }

    void wifiAutoConfig()
    {
        m_wifiStatusDecay = false;
        m_wifiLed->setColor(100,0,100);
        m_wifiStatusColor = *m_wifiLed;
    }

    void wifiOK()
    {
        *m_wifiLed = m_wifiStatusColor;
    }

    void wifiError()
    {
        m_wifiStatusDecay = false;
        m_wifiLed->setColor(100,0,0);
    }

    void extraLedOK()
    {
        m_extraLed->setColor(0,200,0);
    }

    void extraLedIdle()
    {
        m_extraLed->setColor(0,0,200);
    }

    void extraLedError()
    {
        m_extraLed->setColor(200,0,0);
    }

    void extraLedSetDecay(bool decay)
    {
        m_extraLedDecay = decay;
    }

  private:
    ws2812Strip::led*   m_controllerLed;
    ws2812Strip::led*   m_wifiLed;
    ws2812Strip::led*   m_extraLed;
    ws2812Strip::led    m_wifiStatusColor;

    bool                m_statusDecay     = false;
    bool                m_wifiStatusDecay = false;
    bool                m_extraLedDecay   = false;
};


#endif
