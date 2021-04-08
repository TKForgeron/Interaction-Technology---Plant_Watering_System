#include <Arduino.h>
#include "Amux.h"

Amux::Amux(int selectPin, int analogReadPin)
{ // select to LOW: ldr activated, select to HIGH: soil moisture activated
    this->selectPin = selectPin;
    pinMode(selectPin, OUTPUT);
    digitalWrite(selectPin, LOW);

    this->analogReadPin = analogReadPin;
    pinMode(analogReadPin, INPUT);

    this->maxValue = 1024;
};

int Amux::getAnalogValue()
{
    int analogValue = analogRead(this->analogReadPin);
    digitalWrite(this->selectPin, LOW);

    return analogValue;
}

int Amux::getMoistureValue()
{
    digitalWrite(this->selectPin, HIGH);

    return this->maxValue - this->getAnalogValue();
};

int Amux::getLightValue()
{
    return this->getAnalogValue();
};