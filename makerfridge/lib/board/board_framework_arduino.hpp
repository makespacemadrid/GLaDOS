
#ifndef BOARD_FRAMEWORK_ARDUINO_HPP
#define BOARD_FRAMEWORK_ARDUINO_HPP

#include "board_framework.hpp"
#include <Arduino.h>

class BoardFrameworkArduino : public BoardFramework
{
    public:
        virtual ~BoardFrameworkArduino() = default;
        void pinmode(int pin, PIN_MODE mode) const {
            switch (mode) {
                case PIN_MODE::IN:
                    pinMode(pin, INPUT);
                    break;
                case PIN_MODE::OUT:
                    pinMode(pin, OUTPUT);
                    break;
                case PIN_MODE::IN_PULLUP:
                    pinMode(pin, INPUT_PULLUP);
                    break;
                default:
                    Serial.println("ERROR: UNDEFINED PINMODE");
            }

        };
        void write(int pin, int state) const { digitalWrite(pin, state); }
        int read(int pin) const { return digitalRead(pin); }
        void log(const char* message) const { Serial.print(message); }
        void fdelay(int ms) const { return delay(ms); }
};

#endif
