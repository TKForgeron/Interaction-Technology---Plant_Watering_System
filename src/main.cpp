#include <Arduino.h>
#include "BinaryActuator/Led/Led.h"


Led builtInLed(D0);
int sensorValue;
int sensorPin = A0;

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
}
 
void loop() {

  sensorValue = analogRead(sensorPin); 
  Serial.print("Analog Value : ");
  Serial.println(sensorValue);
  
}