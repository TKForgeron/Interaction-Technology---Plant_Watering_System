#include <Arduino.h>
#include "BinarySwitch.h"

BinarySwitch::BinarySwitch(int pinNumber){
    this->pinNumber = pinNumber;    // Set attribute pinNumber to value passed to constructor
    pinMode(pinNumber,INPUT);       // Set up for Sensor use 
};

// Methods
bool BinarySwitch::getState(){
    this->buttonState = !digitalRead(this->pinNumber);
    return this->buttonState;
}

void BinarySwitch::updateState() {
  if (this->buttonState == HIGH) {  // the button has been just pressed
      this->startPressed = millis();
      this->idleTime = this->startPressed - this->endPressed;
  } else {                          // the button has been just released
      this->endPressed = millis();
      this->holdTime = this->endPressed - this->startPressed;
  }

}

/* Records time button is idle/on hold WHILE button is still pressed/released
   usage: to do stuff WHILE button is still being pressed (e.g. showing 'live' timePressed on LCD)*/
void BinarySwitch::updateCounter() {
  if (this->buttonState == HIGH) {  // the button is still pressed
      this->holdTime = millis() - this->startPressed;
  } else {                          // the button is still released
      this->idleTime = millis() - endPressed;
  }
}