// External includes
#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Internal includes
#include "../lib/Amux/Amux.h"
#include "../lib/BinaryActuator/Led/Led.h"
#include "../lib/BinarySwitch/BinarySwitch.h"
#include "../lib/Oled/Oled.h"
#include "../lib/Timer/Timer.h"
#include "../lib/WateringMotor/WateringMotor.h"
#include "../lib/Wifi/Wifi.h"

// MQTT connection
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
unsigned long int value = 0;
const char *mqttServer = "mqtt.uu.nl";
const int mqttPort = 1883;
const char *mqttUser = "student088";
const char *mqttPassword = "JqM5xmPe";

// State enumerations
enum Mode
{
  automatic,
  manual,
  watering
};
enum DisplayState
{
  temperature,
  waterTime,
  humidity,
  moisture,
  pressure
};

// Sensors
BinarySwitch modeButton(D3);
Amux amux(D4, A0); // moisture & light
Adafruit_BME280 bme;

// Actuators
Oled display(0x3c, D7, D5);
WateringMotor wateringMotor(D6);
Led builtInLed(D0);

// Timers
Timer serialPrintTimer;
Timer readSensorTimer;
Timer rotateStateTimer;

// Globals
Mode mode = automatic;
DisplayState displayState = temperature;
unsigned long readSensorDelay = 10000;
unsigned long rotateStateDelay = 5000;
int buttonDebounce = 500;
int lightValue;
int moistureValue;
float temperatureValue;
float humidityValue;
float pressureValue;
int moistureThreshold = 0;
unsigned long lastWaterTime;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void mqttReconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a client ID
    String clientId = "tim";
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("sensor/moisture", "hello world");
      // ... and resubscribe
      client.subscribe("actuator/led");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

void setup()
{
  // Serial monitor (baud)
  Serial.begin(115200);

  // Wifi connection
  Wifi wifi("Openhuis", "qzxvw123");

  pinMode(LED_BUILTIN, OUTPUT);

  // MQTT connection
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  // BME init
  Wire.begin(D7, D5);
  unsigned status = bme.begin(0x76);

  // Intial Values
  lightValue = amux.getLightValue();
  moistureValue = amux.getMoistureValue();
  temperatureValue = bme.readTemperature();
  humidityValue = bme.readHumidity();
  pressureValue = bme.readPressure() / 100.0F;

  serialPrintTimer.start(1000);
  readSensorTimer.start(readSensorDelay);
  rotateStateTimer.start(rotateStateDelay);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

void loop()
{
  if (!client.connected())
  {
    mqttReconnect();
  }
  client.loop();

  if (serialPrintTimer.hasExpired())
  {
    serialPrintTimer.repeat();
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "test value #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("sensor/moisture", msg);

    Serial.println("");
    Serial.println("lightValue: " + String(amux.getLightValue()));
    Serial.println("moistureValue: " + String(amux.getMoistureValue()));
    Serial.println("------------------------");
    Serial.println("");
    Serial.println("");
  }

  if (readSensorTimer.hasExpired())
  {
    readSensorTimer.repeat();

    lightValue = amux.getLightValue();
    moistureValue = amux.getMoistureValue();
    temperatureValue = bme.readTemperature();
    humidityValue = bme.readHumidity();
    pressureValue = bme.readPressure() / 100.0F;
    lastWaterTime = wateringMotor.getLastWaterTime();
  }

  if (rotateStateTimer.hasExpired())
  {
    rotateStateTimer.repeat();

    switch (displayState)
    {
    case temperature:
      displayState = waterTime;
      break;
    case waterTime:
      displayState = moisture;
      break;
    case moisture:
      displayState = pressure;
      break;
    case pressure:
      displayState = humidity;
      break;
    case humidity:
      displayState = temperature;
      break;
    default:
      displayState = waterTime;
      break;
    }

    display.clearLine(0, 20);
  }

  if (mode == automatic)
  {
    if (moistureValue < moistureThreshold)
    {
      display.drawString(0, 44, "Watering...");
      display.display();
      wateringMotor.giveWater(120, 10000);
      display.clearLine(44, 20);
    }

    builtInLed.on();

    if (modeButton.getState())
    {
      if (millis() >= modeButton.lastPressed + buttonDebounce)
      {
        mode = manual;
        modeButton.lastPressed = millis();
      }
    }
  }
  else if (mode == manual)
  {
    builtInLed.off();

    if (modeButton.getState())
    {
      if (millis() >= modeButton.lastPressed + buttonDebounce)
      {
        mode = automatic;
        modeButton.lastPressed = millis();
      }
    }
  }

  switch (displayState)
  {
  case temperature:
    display.drawString(0, 0, "Temperature: " + String(temperatureValue, 1) + " *C");
    wateringMotor.setLastWaterTimeInCorrectUnit(); // it is done here, so when in waterTime state, displayed time does not change
    break;
  case waterTime:
    display.drawString(0, 0, wateringMotor.lastWaterTimeInCorrectUnit + " ago");
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
  default:
    display.drawString(0, 0, "Watering system running");
    break;
  }

  display.display();
}
