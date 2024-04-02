#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(unsigned int pin, void (*shortCallback)(), void (*longCallback)()) : buttonPin(pin), shortPressCallback(shortCallback), longPressCallback(longCallback) {
  pinMode(buttonPin, INPUT);
}

void ButtonHandler::update() {
  if (digitalRead(buttonPin) == HIGH) { // Assuming HIGH means pressed
    unsigned long pressStartTime = millis(); // Record the time when the button was pressed
    while (digitalRead(buttonPin) == HIGH) { // Wait while the button is still pressed
      // Check if the button has been pressed long enough for a long press
      if (millis() - pressStartTime >= longPressTime) {
        Serial.println("Long press detected");
        // Wait for the button to be released
        while(digitalRead(buttonPin) == HIGH) {}
        longPressCallback();
        return; // Return after handling long press to avoid detecting it as a short press
      }
    }
    // If we reach here, the button was released before the longPressTime threshold
    Serial.println("Short press detected");
    shortPressCallback();
  }
}


