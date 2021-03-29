#include <Arduino.h>
#include "MoistureSensor.h"

MoistureSensor::MoistureSensor(int inputPinNumber, int outputPinNumber)
{

    this->inputPinNumber = inputPinNumber;
    pinMode(inputPinNumber, INPUT);

    this->outputPinNumber = outputPinNumber;   
    pinMode(outputPinNumber, OUTPUT);

    this->maxValue = 1024;         
};

int MoistureSensor::getValue()
{

    digitalWrite(this->outputPinNumber, HIGH);
    int analogValue = analogRead(this->inputPinNumber);
    digitalWrite(this->outputPinNumber, LOW);
    return this->maxValue - analogValue;
};