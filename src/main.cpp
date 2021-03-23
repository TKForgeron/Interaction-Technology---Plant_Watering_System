#include <Arduino.h>
#include "../lib/BinaryActuator/Led/Led.h"
#include "../lib/MoistureSensor/MoistureSensor.h"


Led builtInLed(D0);
MoistureSensor moistureSensor(A0, D1);

void setup() {
  Serial.begin(9600);
}
 
void loop() {

  Serial.print("Analog Value : ");
  Serial.println(moistureSensor.getValue());
  
}