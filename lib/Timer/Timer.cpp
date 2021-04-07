#include <Arduino.h>
#include "Timer.h"

Timer::Timer()
{
    // Initializing object
    this->running = false;
    this->startTime = 0;
    this->msDelay = 0;
}

void Timer::start(unsigned long delay)
{
    this->msDelay = delay;      // set object attribute to the desired/demanded delay
    this->startTime = millis(); // record at what point the timer was started
    this->running = true;       // mark this object as running
}

void Timer::stop()
{
    running = false; // mark this object as not running
}

void Timer::repeat()
{
    running = true;                  // mark this object as running
    startTime = startTime + msDelay; // update startTime, so timer is not expired yet
}

// returns true when delay/timer has expired, while it was running, and stops timer in that case
bool Timer::hasExpired()
{
    unsigned long currentTime = millis();
    if (running && ((currentTime - this->startTime) >= this->msDelay))
    {
        this->stop();
        return true;
    }
    else
    {
        return false;
    }
}