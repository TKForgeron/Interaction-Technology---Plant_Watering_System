#include <Arduino.h>

#ifndef INFOB3IT_MOISTURESENSOR_H
#define INFOB3IT_MOISTURESENSOR_H

class MoistureSensor
{

private:
    int inputPinNumber;
    int outputPinNumber;
    int maxValue;

public:
    // Constructor
    MoistureSensor(int inputPinNumber, int outputPinNumber);
    // Methods
    int getValue();
};

#endif //INFOB3IT_MOISTURESENSOR_H
