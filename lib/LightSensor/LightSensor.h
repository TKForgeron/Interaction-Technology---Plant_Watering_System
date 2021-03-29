#include <Arduino.h>

#ifndef INFOB3IT_LIGHTSENSOR_H
#define INFOB3IT_LIGHTSENSOR_H

class LightSensor {

private:
    int pinNumber;

public:
    // Constructor
    LightSensor(int pinNumber);
    // Methods
    int getValue();
    bool thresholdReached(int threshold = 500);
};

#endif //INFOB3IT_LIGHTSENSOR_H






