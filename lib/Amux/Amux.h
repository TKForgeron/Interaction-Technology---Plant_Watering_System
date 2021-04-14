#include <Arduino.h>

#ifndef INFOB3IT_AMUX_H
#define INFOB3IT_AMUX_H

class Amux
{
private:
    int selectPin;
    int analogReadPin;
    float getAnalogValue();

public:
    // Constructor
    Amux(int selectPin, int analogReadPin);
    // Methods
    float getMoistureValue();
    float getLightValue();
};

#endif //INFOB3IT_AMUX_H