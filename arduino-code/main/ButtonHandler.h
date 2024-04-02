#ifndef ButtonHandler_h
#define ButtonHandler_h

#include <Arduino.h>

class ButtonHandler {
private:
  unsigned int buttonPin;
  const unsigned long debounceDelay = 50;
  const unsigned long longPressTime = 3000;
  unsigned long lastDebounceTime = 0;
  bool lastButtonState = 0;
  unsigned long buttonPressStartTime = 0;
  
  void (*shortPressCallback)();
  void (*longPressCallback)();

public:
  ButtonHandler(unsigned int pin, void (*shortPressCallback)(), void (*longPressCallback)());
  void update();
};

#endif
