#include <Arduino.h>
#include <Adafruit_BME280.h>

#include "../lib/BinaryActuator/Led/Led.h"
#include "../lib/MoistureSensor/MoistureSensor.h"
#include "../lib/BinarySwitch/BinarySwitch.h"
#include "../lib/Timer/Timer.h"
#include "../lib/Oled/Oled.h"
#include "../lib/WateringMotor/WateringMotor.h"

// State enumerations
enum Mode{automatic, manual, watering};
enum DisplayState{temperature, humidity, moisture, pressure, waterTime};

// Sensors
BinarySwitch modeButton(D3);
MoistureSensor moistureSensor(A0, D4);
Adafruit_BME280 bme;

// Actuators
Oled display(0x3c, D7, D5);
WateringMotor wateringMotor(D6);
Led builtInLed(D0);

// // Timers
Timer readSensorTimer;
Timer rotateStateTimer;

// // Globals
Mode mode = automatic;
DisplayState displayState = temperature;
unsigned long readSensorDelay = 4000;
unsigned long rotateStateDelay = 2000;
int buttonDebounce = 500;
int moistureValue;
float temperatureValue;
float humidityValue;
float pressureValue;
int moistureThreshold = 500;
unsigned long lastWaterTime;

void setup() {
  // BME init
  Wire.begin(D7, D5);
  unsigned status = bme.begin(0x76);
 
  // Intial Values 
  moistureValue = moistureSensor.getValue();
  temperatureValue = bme.readTemperature();
  humidityValue = bme.readHumidity();
  pressureValue = bme.readPressure() / 100.0F;

  Serial.begin(9600);
  readSensorTimer.start(readSensorDelay);
  rotateStateTimer.start(rotateStateDelay);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

 
void loop() {

  Serial.println(moistureValue);
  Serial.println(wateringMotor.getLastWaterTime());

  if(readSensorTimer.hasExpired()){
    
    moistureValue = moistureSensor.getValue();
    temperatureValue = bme.readTemperature();
    humidityValue = bme.readHumidity();
    pressureValue = bme.readPressure() / 100.0F;
    lastWaterTime = wateringMotor.getLastWaterTime();
    
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
          displayState = waterTime;
          break;
      case waterTime:
          displayState = temperature;
          break;  
      default:
          displayState = temperature;
          break;
    }

    display.clearLine(0, 20);
    rotateStateTimer.repeat();
  
  }

  if(mode == automatic){

    if (moistureValue < moistureThreshold){
      display.drawString(0, 44, "Watering...");
      display.display();
      wateringMotor.giveWater(120, 10000);
      display.clearLine(44, 20);
    }

    builtInLed.on();
    
    if(modeButton.getState()){
      if(millis() >= modeButton.lastPressed + buttonDebounce){
            mode = manual;
            modeButton.lastPressed = millis();
      }

      
    }
     
  }
  else if (mode == manual){

    builtInLed.off();

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
          display.drawString(0, 0, "Moisture: " + String(moistureValue));
          break;
      case pressure:
          display.drawString(0, 0, "Pressure: " + String(pressureValue, 0) + " hPa");
          break;
      case waterTime:
          display.drawString(0, 0, String(lastWaterTime / 1000) + "s ago");
          break;
      default:
          display.drawString(0, 0, "Watering system running");
          break;
  }
  
  display.display();

}


