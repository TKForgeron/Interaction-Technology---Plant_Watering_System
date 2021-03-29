#include <Arduino.h>
#include "WateringMotor.h"

WateringMotor::WateringMotor(int pinNumber) : Servo() {
    this->pinNumber = pinNumber;
    this->lastWaterTime = 0;
}

void WateringMotor::giveWater(int angle, int duration) {

  this->lastWaterTime = millis();

  this->attach(D6);

  for (int pos = angle; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    this->write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  delay(duration); 
  
  for (int pos = 0; pos <= angle; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    this->write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }

  this->detach();

}

unsigned long WateringMotor::getLastWaterTime(){
    return millis() - this->lastWaterTime;
}
