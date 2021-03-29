#include <Arduino.h>
#include "MoistureSensor.h"

MoistureSensor::MoistureSensor(int inputPinNumber, int outputPinNumber)
{

    this->inputPinNumber = inputPinNumber;
    pinMode(inputPinNumber, INPUT);

<<<<<<< HEAD
    this->outputPinNumber = outputPinNumber;   
    pinMode(outputPinNumber, OUTPUT);

    this->maxValue = 1024;         
=======
    this->outputPinNumber = outputPinNumber;
    pinMode(outputPinNumber, OUTPUT);
>>>>>>> 6da70717e4d4d6731444439b50bb5c3769627fe0
};

int MoistureSensor::getValue()
{

    digitalWrite(this->outputPinNumber, HIGH);
    int analogValue = analogRead(this->inputPinNumber);
    digitalWrite(this->outputPinNumber, LOW);
    return this->maxValue - analogValue;
};