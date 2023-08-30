#include <Servo.h>

#define S1min 10
#define S1max 160
#define DEF1 100
#define S2min 30
#define S2max 130
#define DEF2 90
#define S3min 90
#define S3max 160
#define DEF3 120
#define S4min 0
#define S4max 180
#define DEF4 90

#define body_pin 4
#define neck_pin 5
#define head_pin 6
#define base_pin 11


String str;
String str_servo;
int p;
int answer=0;

int redPin= 7;
int greenPin = 6;
int bluePin = 5;

class Robot_Part
{
  private:
    Servo servo;
    uint8_t servo_pin;

  public:
    uint8_t min_pos;
    uint8_t max_pos;
    uint8_t target;
    Robot_Part(uint8_t pin,uint8_t def,uint8_t min,uint8_t max)
    {
        servo_pin=pin;
        target=def;
        min_pos=min;
        max_pos=max;
    }

    void set()
    {
      servo.attach(servo_pin);
      servo.write(target);
      delay(500);
    }

    void move()
    {
      
      if (servo.read() < target)
      {
        servo.write(servo.read()+1);
      }
      else if (servo.read() > target)
      {
        servo.write(servo.read()-1);
      }
      delay(3);
    }
};




Robot_Part body(body_pin,DEF1,S1min,S1max);
Robot_Part neck (neck_pin,DEF2,S2min,S2max);
Robot_Part head (head_pin,DEF3,S3min,S3max);
Robot_Part base (base_pin,DEF4,S4min,S4max);

void serial();















void setup() {  
  Serial.begin(115200);
  body.set();
  neck.set();
  head.set();
  base.set();
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  Serial.println("init");
}



void loop() 
{
  serial();
  body.move();
  neck.move();
  head.move();
  base.move();

  if (answer==1)
  {
    neck.target=random(30,130);
    head.target=random(90,160);
  }


  
}





//Example : servo1:170
void serial()
{
   if (Serial.available())
  {
    str=Serial.readStringUntil('\n');
    Serial.println(str);
    str_servo=str.substring(0,6);
    p=str.substring(7,10).toInt();
    //yield(); avant print
    
    if(str_servo=="servo0")
    {
        
    }
    else if(str_servo=="servo1")
    {
      if(p < body.min_pos)
      {
        p=body.min_pos;
      }
      if(p > body.max_pos)
      {
        p=body.max_pos;
      }
      body.target=p;
    }
    else if(str_servo=="servo2")
    {
      if(p < neck.min_pos)
      {
        p=neck.min_pos;
      }
      if(p > neck.max_pos)
      {
        p=neck.max_pos;
      }
      neck.target=p;
    }
    else if(str_servo=="servo3")
    {
      if(p < head.min_pos)
      {
        p=head.min_pos;
      }
      if(p > head.max_pos)
      {
        p=head.max_pos;
      }
      head.target=p;
    }
    else if(str_servo=="servo4")
    {
      if(p < base.min_pos)
      {
        p=base.min_pos;
      }
      if(p > base.max_pos)
      {
        p=base.max_pos;
      }
      base.target=p;
    }
    else if(str_servo=="servox")
    {
       base.target=p/5;
       
    }
    else if(str_servo=="servoy")
    {
      if(p < body.min_pos)
      {
        p=body.min_pos;
      }
      if(p > body.max_pos)
      {
        p=body.max_pos;
      }
      body.target=p/5;      
    }
    else if(str_servo=="servoa")
    { 
       if (p==666)
       {
          answer=1;
          //setColor(170, 0, 255); // Purple Color : answer
       }
       if (p==999)
       {
          answer=0;
          //setColor(255, 0, 0); // Red Color : Nothing
       }
    }
    else if(str_servo=="servot")
    { 
       if (p==666)
       {
          //setColor(255, 255 255); // Purple Color : thinking
       }
       if (p==999)
       {
          //setColor(255, 0, 0); // Red Color : Nothing
       }
    }
    else if(str_servo=="servol")
    { 
       if (p==666)
       {
          //setColor(0, 255, 0); // Purple Color : answer
       }
       if (p==999)
       {
          //setColor(255, 0, 0); // Red Color : Nothing
       }
    }
  }
}
/*
void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redPin, redValue);
  analogWrite(greenPin, greenValue);
  analogWrite(bluePin, blueValue);
}
*/