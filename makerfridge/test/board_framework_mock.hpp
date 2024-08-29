#ifndef BOARDFRAMEWORKMOCK_HPP
#define BOARDFRAMEWORKMOCK_HPP

#include <stdio.h>

#include "board_framework.hpp"

class BoardFrameworkMock : public BoardFramework {
    public:
        virtual ~BoardFrameworkMock() = default;
        void pinmode(int pin, PIN_MODE mode) const {
            switch (mode) {
                case PIN_MODE::IN:
                    printf("Setting pinmode for pin %d to %s\n", pin, "INPUT");
                    break;
                case PIN_MODE::OUT:
                    printf("Setting pinmode for pin %d to %s\n", pin, "OUTPUT");
                    break;
                case PIN_MODE::IN_PULLUP:
                    printf("Setting pinmode for pin %d to %s\n", pin, "INPUT_PULLUP");
                    break;
                default:
                    printf("ERROR: UNDEFINED PINMODE");
            }
        };
        void write(int pin, int state) const {
            printf("Writting to pin %d the state %d\n", pin, state);
        }
        int read(int pin) const {
            int pin_state = HIGH;
            if (pin == 25) {
                // This will only return that the product 3 button is pressed.
                pin_state = LOW;
            }
            printf("Reading from pin %d the state %d\n", pin, pin_state);
            return pin_state;
        }
        void log(const char* message) const { printf("%s", message); }
        void fdelay(int ms) const {
            printf("Sleep for %d (not really)\n", ms); 
        }
};

#endif // BOARDFRAMEWORKMOCK_HPP
