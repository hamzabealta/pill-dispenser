#ifndef LEDController_h
#define LEDController_h

#include <Arduino.h>

class LEDController {
private:
    unsigned int ledPin;

public:
    LEDController(unsigned int pin);
    void turnOn();
    void turnOff();
    void toggle(); 
};

#endif
