#include <Arduino.h>

#ifndef INFOB3IT_AMUX_H
#define INFOB3IT_AMUX_H

class Amux
{
private:
    int selectPin;
    int analogReadPin;
    int maxValue;
    int getAnalogValue();

public:
    // Constructor
    Amux(int selectPin, int analogReadPin);
    // Methods
    int getMoistureValue();
    int getLightValue();
};

#endif //INFOB3IT_AMUX_H