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
    int moist = this->getAnalogValue();
    digitalWrite(this->selectPin, HIGH);
    Serial.print("moist: ");
    Serial.println(moist);
    return this->maxValue - moist;
};

int Amux::getLightValue()
{
    Serial.print("light: ");
    Serial.println(this->getAnalogValue());
    return this->getAnalogValue();
};