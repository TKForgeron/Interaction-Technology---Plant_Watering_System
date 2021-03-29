/*
    inspiration taken from: https://github.com/PowerBroker2/SafeString
*/

#include <Arduino.h>

#ifndef INFOB3IT_TIMER_H
#define INFOB3IT_TIMER_H

class Timer{ 

private:
    // Primitive types, therefore no reference
    unsigned long msDelay;      // desired delay in milliseconds
    unsigned long startTime;    // point the timer was started
    bool running;               // true whenever timer running, false when ended

public:
    // Constructor
    Timer();
    // Methods
    void start(unsigned long delay);
    void stop();
    void repeat();
    bool hasExpired(); // records whether the timer has just changed state from 'notExpired' to 'expired'
};

#endif //INFOB3IT_TIMER_H