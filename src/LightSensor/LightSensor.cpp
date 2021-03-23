#include <Arduino.h>
#include "LightSensor.h"


LightSensor::LightSensor(int pinNumber){
    this->pinNumber = pinNumber;    // Set attribute pinNumber to value passed to constructor
    pinMode(pinNumber,INPUT);       // Set up for Sensor use
};

int LightSensor::getValue(){
    return analogRead(this->pinNumber);
};

bool LightSensor::thresholdReached(int threshold){
    return this->getValue() > threshold;   
}