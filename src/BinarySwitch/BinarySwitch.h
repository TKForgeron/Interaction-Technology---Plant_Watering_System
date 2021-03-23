#include <Arduino.h>

#ifndef INFOB3IT_BINARYSWITCH_H
#define INFOB3IT_BINARYSWITCH_H

class BinarySwitch{ 

private:
    // Primitive type therefore no reference
    unsigned long  startPressed = 0;    // the moment the button was pressed
    unsigned long  endPressed = 0;      // the moment the button was released
    int pinNumber;
public:
    // constructor
    BinarySwitch(int pinNumber);
    // Methods
    bool getState();
    void updateState();
    void updateCounter();
    // Properties
    unsigned long  holdTime = 0;    // how long the button was hold
    unsigned long idleTime = 0;     // how long the button was idle
    int buttonState = 0;            // current state of the button
    int lastButtonState = 0;        // previous state of the button
    unsigned long lastPressed = 0;
};

#endif //INFOB3IT_BINARYSWITCH_H