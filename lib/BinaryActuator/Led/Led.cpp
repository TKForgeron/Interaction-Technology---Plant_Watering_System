#include <Arduino.h>
#include "../BinaryActuator.h"
#include "Led.h"

Led::Led(int pinNumber) : BinaryActuator(pinNumber) {
};

void Led::flash(int amount, int flashSpeed){    
    // Loop and flash
    for(int i=0; i < amount; i++){
        this->on();
        delay(flashSpeed);
        this->off();
        delay(flashSpeed);
    }

};