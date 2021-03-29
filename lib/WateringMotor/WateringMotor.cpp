#include <Arduino.h>
#include "WateringMotor.h"
#include "../Timer/Timer.h"
#include "../Stopwatch/Stopwatch.h"

WateringMotor::WateringMotor(int pinNumber) : Servo()
{
  this->pinNumber = pinNumber;
  this->lastWaterTime = 0;
}

void WateringMotor::giveWater(int angle, int duration)
{
  Stopwatch durationStopwatch;
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

  durationStopwatch.start();
  unsigned long timeWaiting = 0;
  while (timeWaiting < duration)
  {
    timeWaiting = durationStopwatch.getTime();
  }
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
