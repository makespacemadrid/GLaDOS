#include <Servo.h>

#define S1min 10
#define S1max 160
#define DEF1 100
#define S2min 30
#define S2max 130
#define DEF2 80
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


class Robot_Part
{
  private:
    Servo servo;
    uint8_t min_pos;
    uint8_t max_pos;
    uint8_t default_pos;
    uint8_t servo_pin;

  public:
    Robot_Part(uint8_t pin,uint8_t def,uint8_t min,uint8_t max)
    {
        servo_pin=pin;
        default_pos=def;
        min_pos=min;
        max_pos=max;
    }

    void move(uint8_t pos)
    {
      if(pos < min_pos)
      {
        pos=min_pos;
      }
      if(pos>=max_pos)
      {
        pos=max_pos;
      }
      servo.write(pos);
    }

    void set()
    {
      servo.attach(servo_pin);
      servo.write(default_pos);
      delay(500);
    }
};




Robot_Part body(body_pin,DEF1,S1min,S1max);
Robot_Part neck (neck_pin,DEF2,S2min,S2max);
Robot_Part head (head_pin,DEF3,S3min,S3max);
Robot_Part base (base_pin,DEF4,S4min,S4max);



void all_move();
void scene1();
void serial();
void bodym();
void up();














void setup() {  
  Serial.begin(115200);
  body.set();
  neck.set();
  head.set();
  base.set();
}



void loop() 
{
  //all_move();
  //scene1();
  //serial();
  //bodym();
  up();
}











 void all_move()
 {
    for(int i=0; i<=180 ;i++)
    {
      body.move(i);
      neck.move(i);
      head.move(i);
      base.move(i);
      delay(10);
    }
    for(int i=180; i>=0 ;i--)
    {
      body.move(i);
      neck.move(i);
      head.move(i);
      base.move(i);
      delay(10);
    }
 }






 void scene1()
 {
   //initial position
   body.move(180);
   delay(15);
   head.move(0);
   delay(15);
   neck.move(80);
   delay(15);

   //see people
   for(int i=0; i<=180 ;i++)
    {
      head.move(i);
      delay(10);
    }
    delay(3000);
    for(int i=180; i>=0 ;i--)
    {
      head.move(i);
      delay(10);
    }
    //up
    for(int i=180; i>=0 ;i--)
    {
      body.move(i);
      delay(25);
    }
    //say no
    for(int i=80; i<=130 ;i=i+2)
    {
      neck.move(i);
      delay(10);
    }
    for(int i=130; i>=30 ;i=i-2)
    {
      neck.move(i);
      delay(10);
    }
    for(int i=30; i<=80 ;i=i+2)
    {
      neck.move(i);
      delay(10);
    }
    delay(1000);
    //up head
    for(int i=0; i<=180 ;i++)
    {
      head.move(i);
      delay(10);
    }
    delay(2000);
    //down
    for(int i=0; i<=180 ;i++)
    {
      body.move(i);
      delay(25);
    }
    delay(1500);
    //shutdown all
    for(int i=180; i>=0 ;i=i-20)
    {
      head.move(i);
      delay(10);
    }
    delay(5000);
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
        up();
    }
    else if(str_servo=="servo1")
    {
        body.move(p);
        delay(15);
        //str="servo1:"+String(pos);
        //Serial.println(str);
        Serial.println("Body");
    }
    else if(str_servo=="servo2")
    {
        neck.move(p);
        delay(15);
        //str="servo2:"+String(pos);
        //Serial.println(str);
        Serial.println("Neck");
    }
    else if(str_servo=="servo3")
    {
        head.move(p);
        delay(15);
        //str="servo3:"+String(pos);
        //Serial.println(str);
        Serial.println("Head");
    }
    else if(str_servo=="servo4")
    {
        base.move(p);
        delay(15);
        //str="servo3:"+String(pos);
        //Serial.println(str);
        Serial.println("Base");
    }
  }
  delay(500); 
}





void bodym()
{
    for(int i=0; i<=180 ;i++)
    {
      body.move(i);
      delay(10);
    }
    delay(500);
    for(int i=180; i>=0 ;i--)
    {
      body.move(i);
      delay(10);
    }
    delay(500);
}


void up()
{
  body.move(0);
  head.move(180);
  neck.move(DEF2);
  Serial.println("UP!!! Ready to Listen you");
}
