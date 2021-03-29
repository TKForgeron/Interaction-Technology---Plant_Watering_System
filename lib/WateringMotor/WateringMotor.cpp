#include <Arduino.h>
#include "WateringMotor.h"
#include "../Timer/Timer.h"

WateringMotor::WateringMotor(int pinNumber) : Servo()
{
  this->pinNumber = pinNumber;
  this->lastWaterTime = 0;
}

void WateringMotor::giveWater(int angle, int duration)
{
  Timer positionReachTimer;
  positionReachTimer.start(15);

  this->attach(this->pinNumber);
  int pos = angle;
  while (pos >= 0)
  { // goes from 180 degrees to 0 degrees
    if (positionReachTimer.hasExpired())
    {
      this->write(pos);
      pos -= 1;
      positionReachTimer.repeat();
    }
  }

  this->lastWaterTime = millis();

  delay(duration);
  
  while (pos <= angle)
  {
    if (positionReachTimer.hasExpired())
    {
      this->write(pos);
      pos += 1;
      positionReachTimer.repeat();
    }
  }

  this->detach();
}

unsigned long WateringMotor::getLastWaterTime()
{
  return millis() - this->lastWaterTime;
}
