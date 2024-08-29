#ifndef BOARD_FRAMEWORK_HPP
#define BOARD_FRAMEWORK_HPP

#define HIGH 0x1
#define LOW 0x0

enum PIN_MODE {
    OUT,
    IN,
    IN_PULLUP,
};
/***
 * The purpose of this structure is to decouple the controller framework
 * from the machine state in order to make independent tests.
 */
class BoardFramework {
public:
    virtual ~BoardFramework() = default;
    virtual void pinmode(int pin, PIN_MODE mode) const = 0;
    virtual void write(int pin, int state) const = 0;
    virtual int read(int pin) const = 0;
    virtual void log(const char* message) const = 0;
    virtual void fdelay(int ms) const = 0;
};

#endif
