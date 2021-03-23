#include <Arduino.h>

#ifndef INFOB3IT_STOPWATCH_H
#define INFOB3IT_STOPWATCH_H

class Stopwatch{ 

private:

    unsigned long startTime;     
    unsigned long stopTime;
    bool isCounting;            

public:
    // Constructor
    Stopwatch();
    // Methods
    unsigned long getTime();
    void start();
    void stop();
    void reset(); 
};

#endif //INFOB3IT_STOPWATCH_H

