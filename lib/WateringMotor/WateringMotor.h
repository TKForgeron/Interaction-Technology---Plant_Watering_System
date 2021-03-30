#include <Arduino.h>
#include <Servo.h>

#ifndef INFOB3IT_WATERINGMOTOR_H
#define INFOB3IT_WATERINGMOTOR_H

class WateringMotor : public Servo
{

private:
    int pinNumber;
    unsigned long lastWaterTime;

public:
    // Constructor
    WateringMotor(int pinNumber);

    // Methods
    void giveWater(int angle, int duration);
    unsigned long getLastWaterTime();
    void setLastWaterTimeInCorrectUnit();

    // Attributes
    String lastWaterTimeInCorrectUnit;
};

#endif //INFOB3IT_WATERINGMOTOR_H