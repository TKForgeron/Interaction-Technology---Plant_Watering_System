#include <Arduino.h>
#include "../BinaryActuator.h"

#ifndef INFOB3IT_LED_H
#define INFOB3IT_LED_H

class Led : public BinaryActuator { 

public:
    // Constructor
    Led(int pinNumber);
    // Methods
    void flash(int amount = 1, int flashSpeed = 250);
};

#endif //INFOB3IT_LED_H