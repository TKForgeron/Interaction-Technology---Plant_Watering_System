#include <Arduino.h>
#include "Amux.h"

Amux::Amux(int selectPin, int analogReadPin)
{ // select to LOW: ldr activated, select to HIGH: soil moisture activated
    this->selectPin = selectPin;
    pinMode(selectPin, OUTPUT);
    digitalWrite(selectPin, LOW);

    this->analogReadPin = analogReadPin;
    pinMode(analogReadPin, INPUT);
};

float Amux::getAnalogValue()
{
    float analogValue = (float)analogRead(this->analogReadPin);
    digitalWrite(this->selectPin, LOW);

    return analogValue / 1023.0F * 100.0F;
}

float Amux::getMoistureValue()
{
    digitalWrite(this->selectPin, HIGH);
    return this->getAnalogValue();
};

float Amux::getLightValue()
{
    return this->getAnalogValue();
};