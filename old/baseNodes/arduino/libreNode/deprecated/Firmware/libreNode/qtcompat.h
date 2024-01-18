#ifndef QTCOMPAT_H
#define QTCOMPAT_H

#ifndef ESP8266
#ifndef __AVR_ATmega2560__
//QT creator compatibilidad
#include <QObject>
#include <vector>
typedef quint16 uint16_t;
typedef quint8  uint8_t;
//typedef QString String;

#define A0 0
#define D0 0
#define D1 0
#define D2 0
#define D3 0
#define D4 0
#define D5 0
#define D6 0
#define D7 0
#define D8 0

#define NEO_MATRIX_TOP         0x00 // Pixel 0 is at top of matrix
#define NEO_MATRIX_BOTTOM      0x01 // Pixel 0 is at bottom of matrix
#define NEO_MATRIX_LEFT        0x00 // Pixel 0 is at left of matrix
#define NEO_MATRIX_RIGHT       0x02 // Pixel 0 is at right of matrix
#define NEO_MATRIX_CORNER      0x03 // Bitmask for pixel 0 matrix corner
#define NEO_MATRIX_ROWS        0x00 // Matrix is row major (horizontal)
#define NEO_MATRIX_COLUMNS     0x04 // Matrix is column major (vertical)
#define NEO_MATRIX_AXIS        0x04 // Bitmask for row/column layout
#define NEO_MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
#define NEO_MATRIX_ZIGZAG      0x08 // Pixel order reverses between lines
#define NEO_MATRIX_SEQUENCE    0x08 // Bitmask for pixel line order

// These apply only to tiled displays (multiple matrices):

#define NEO_TILE_TOP           0x00 // First tile is at top of matrix
#define NEO_TILE_BOTTOM        0x10 // First tile is at bottom of matrix
#define NEO_TILE_LEFT          0x00 // First tile is at left of matrix
#define NEO_TILE_RIGHT         0x20 // First tile is at right of matrix
#define NEO_TILE_CORNER        0x30 // Bitmask for first tile corner
#define NEO_TILE_ROWS          0x00 // Tiles ordered in rows
#define NEO_TILE_COLUMNS       0x40 // Tiles ordered in columns
#define NEO_TILE_AXIS          0x40 // Bitmask for tile H/V orientation
#define NEO_TILE_PROGRESSIVE   0x00 // Same tile order across each line
#define NEO_TILE_ZIGZAG        0x80 // Tile order reverses between lines
#define NEO_TILE_SEQUENCE      0x80 // Bitmask for tile line order


void yield();

void delay(int ms);
void pinMode(int pin, int mode);
int  digitalRead(int pin);
int  analogRead(int pin);
void digitalWrite(int pin, bool state);
void analogWrite(int pin, int pwm);


struct CRGB
{//Tratar de incluir la de la libreria
    CRGB(uint8_t r=0,uint8_t g=0,uint8_t b=0) : r(r),g(g),b(b) {;}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};



class WiFiClient
{
public:
    WiFiClient();

};

#include <QString>
class String : public QString
{
public:
    char charAt(int i)
    {
        return this->at(i).toLatin1();
    }
    String substring(uint16_t pos, uint16_t count = -1)
    {
        String result;
        result.append(this->mid(pos,count));
        return result;
    }

    void toCharArray(char* result, uint16_t count)
    {
        for(int i = 0 ; i < count ; i++)
        {
            if(i < this->length())
                result[i] = charAt(i);
            else
                result[i] = 0;
        }
    }
};
/*
class String
{
public:
    String();
    String(char*);
    void charAt(int i);
    void endsWith(String);
    void equals(String);
    void indexOf(String);
    int length();
    void remove();
    bool startsWith();
    String substring(int start, int end);
    void toCharArray(char* c, int count);
    void toInt();
    void toFloat();
    String toLowerCase();
    String toUpperCase();
    String trim();
};
*/
class Serial
{
public:
    Serial();
    void begin(int baud);
    char read();
    void print();
    void println();
};

//uint16_t timeSinceLastFrameMS = 20;
//uint16_t thisCycleMS = 0;

#endif
#endif


#endif // QTCOMPAT_H

