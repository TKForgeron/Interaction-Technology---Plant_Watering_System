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

  yield();
  delay(duration);
  yield();

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

void WateringMotor::setLastWaterTimeInCorrectUnit()
{
  unsigned int waterTime = (int)(this->getLastWaterTime() / 1000);
  unsigned int waterTimeCorrectedForUnit;
  String unit = " seconds";

  if (waterTime >= 60) // minute
  {
    waterTimeCorrectedForUnit = waterTime / 60;
    if (waterTime >= 60 * 2) // minutes
    {
      unit = " minutes";
    }
    else
    {
      unit = " minute";
    }
  }
  else if (waterTime >= 60 * 60) // hour
  {
    waterTimeCorrectedForUnit = waterTime / (60 * 60);

    if (waterTime >= 60 * 60 * 2) // hours
    {
      unit = " hours";
    }
    else
    {
      unit = " hour";
    }
  }
  else if (waterTime >= 60 * 60 * 24) // day
  {
    waterTimeCorrectedForUnit = waterTime / (60 * 60 * 24);

    if (waterTime >= 60 * 60 * 24 * 2) // days
    {
      unit = " days";
    }
    else
    {
      unit = " day";
    }
  }
  else // if (waterTime < 60) // seconds
  {
    waterTimeCorrectedForUnit = waterTime;
  }

  this->lastWaterTimeInCorrectUnit = String(waterTimeCorrectedForUnit) + unit;
}