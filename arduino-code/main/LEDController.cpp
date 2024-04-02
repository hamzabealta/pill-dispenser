#include "LEDController.h"

LEDController::LEDController(unsigned int pin) : ledPin(pin) {
  pinMode(ledPin, OUTPUT);
}

void LEDController::turnOn() {
  digitalWrite(ledPin, HIGH);
}

void LEDController::turnOff() {
  digitalWrite(ledPin, LOW);
}

void LEDController::toggle() {
  digitalWrite(ledPin, !digitalRead(ledPin));
}
