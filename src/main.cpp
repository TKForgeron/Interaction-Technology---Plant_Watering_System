#include <Arduino.h>
#include <Adafruit_BME280.h>
#include "SSD1306Wire.h"
#include <Servo.h>

#include "../lib/BinaryActuator/Led/Led.h"
#include "../lib/MoistureSensor/MoistureSensor.h"
#include "../lib/BinarySwitch/BinarySwitch.h"
#include "../lib/Timer/Timer.h"

// State enumerations
enum Mode{automatic, manual};
enum DisplayState{temperature, humidity, moisture, pressure};

// Sensors
BinarySwitch modeButton(D3);
MoistureSensor moistureSensor(A0, D4);
Adafruit_BME280 bme;

// Actuators
SSD1306Wire display(0x3c, D7, D5);
Servo servoMotor;
Led builtInLed(D0);

// Timers
Timer readSensorTimer;
Timer rotateStateTimer;

// Globals
Mode mode = automatic;
DisplayState displayState = temperature;
unsigned long readSensorDelay = 10000;
unsigned long rotateStateDelay = 5000;
int buttonDebounce = 1000;
int moistureValue;
float temperatureValue;
float humidityValue;
float pressureValue;

void setup() {
  unsigned status = bme.begin(0x76);
  // Intial Values 
  moistureValue = moistureSensor.getValue();
  temperatureValue = bme.readTemperature();
  humidityValue = bme.readHumidity();
  pressureValue = bme.readPressure();

  Serial.begin(9600);
  readSensorTimer.start(readSensorDelay);
  rotateStateTimer.start(rotateStateDelay);

  servoMotor.attach(D6);

  
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

 
void loop() {

  if(readSensorTimer.hasExpired()){

    moistureValue = moistureSensor.getValue();
    temperatureValue = bme.readTemperature();
    humidityValue = bme.readHumidity();
    pressureValue = bme.readPressure();

    Serial.print("Moisture: ");
    Serial.println(moistureValue);

    Serial.print("Temp: ");
    Serial.println(temperatureValue);

    Serial.print("Humidity: ");
    Serial.println(humidityValue);

    Serial.print("Pressure: ");
    Serial.println(pressureValue);
    
    readSensorTimer.repeat();


  }

  if(rotateStateTimer.hasExpired()){

    switch (displayState) {
      case temperature:
          displayState = humidity;
          break;
      case humidity:
          displayState = moisture;
          break;  
      case moisture:
          displayState = pressure;
          break;
      case pressure:
          displayState = temperature;
          break;
      default:
          displayState = temperature;
          break;
    }

    display.clear();    
    rotateStateTimer.repeat();
  
  }

  if(mode == automatic){
    
    if(modeButton.getState()){
      if(millis() >= modeButton.lastPressed + buttonDebounce){
            mode = manual;
            modeButton.lastPressed = millis();
      }
    }
     
  }
  else if (mode == manual){

    if(modeButton.getState()){
      if(millis() >= modeButton.lastPressed + buttonDebounce){
            mode = automatic;
            modeButton.lastPressed = millis();
      }
    }
    
  }
  
  switch (displayState) {
      case temperature:
          display.drawString(0, 0, "Temperature: " + String(temperatureValue, 1) + " *C");
          break;
      case humidity:
          display.drawString(0, 0, "Humidity: " + String(humidityValue, 0) + "%");
          break;  
      case moisture:
          display.drawString(0, 0, "Moisture: " + String(moistureValue / 1024 * 100) + "%");
          break;
      case pressure:
          display.drawString(0, 0, "Pressure: " + String(pressureValue, 0) + " hPa");
          break;
      default:
          display.drawString(0, 0, "Watering system running");
          break;
  }
  
  
  display.display();

}

