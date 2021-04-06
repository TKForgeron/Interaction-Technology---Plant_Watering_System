// External includes
#include <Arduino.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Internal includes
#include "../lib/BinaryActuator/Led/Led.h"
#include "../lib/MoistureSensor/MoistureSensor.h"
#include "../lib/BinarySwitch/BinarySwitch.h"
#include "../lib/Timer/Timer.h"
#include "../lib/Oled/Oled.h"
#include "../lib/WateringMotor/WateringMotor.h"
#include "../lib/Wifi/Wifi.h"

// MQTT connection
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
const char *mqtt_server = "broker.mqtt-dashboard.com";

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
MoistureSensor moistureSensor(A0, D4);
Adafruit_BME280 bme;

// Actuators
Oled display(0x3c, D7, D5);
WateringMotor wateringMotor(D6);
Led builtInLed(D0);

// Timers
Timer readSensorTimer;
Timer rotateStateTimer;

// Globals
Mode mode = automatic;
DisplayState displayState = temperature;
unsigned long readSensorDelay = 10000;
unsigned long rotateStateDelay = 5000;
int buttonDebounce = 500;
int moistureValue;
float temperatureValue;
float humidityValue;
float pressureValue;
int moistureThreshold = 0;
unsigned long lastWaterTime;

void callback(char *topic, byte *payload, unsigned int length)
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
    digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  // Serial monitor (baud)
  Serial.begin(115200);

  // Wifi connection
  Wifi wifi("Openhuis", "qzxvw123");

  pinMode(BUILTIN_LED, OUTPUT);

  // MQTT connection
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // BME init
  Wire.begin(D7, D5);
  unsigned status = bme.begin(0x76);

  // Intial Values
  moistureValue = moistureSensor.getValue();
  temperatureValue = bme.readTemperature();
  humidityValue = bme.readHumidity();
  pressureValue = bme.readPressure() / 100.0F;

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
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }

  // Serial.println("moistureValue: " + moistureValue);
  // Serial.println("wateringMotor.getLastWaterTime(): " + wateringMotor.getLastWaterTime());

  if (readSensorTimer.hasExpired())
  {
    moistureValue = moistureSensor.getValue();
    temperatureValue = bme.readTemperature();
    humidityValue = bme.readHumidity();
    pressureValue = bme.readPressure() / 100.0F;
    lastWaterTime = wateringMotor.getLastWaterTime();

    readSensorTimer.repeat();
  }

  if (rotateStateTimer.hasExpired())
  {
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
    rotateStateTimer.repeat();
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
