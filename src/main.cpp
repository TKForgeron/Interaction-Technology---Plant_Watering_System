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
char moisturePub[MSG_BUFFER_SIZE];
char pressurePub[MSG_BUFFER_SIZE];
char humidityPub[MSG_BUFFER_SIZE];
char temperaturePub[MSG_BUFFER_SIZE];
char lightPub[MSG_BUFFER_SIZE];
char modePub[MSG_BUFFER_SIZE];
const char *mqttServer = "mqtt.uu.nl";
const int mqttPort = 1883;
const char *mqttUser = "student088";
const char *mqttPassword = "JqM5xmPe";
String clientId = "plantWateringSystem";
const char *mqttWillMessage = "offline";
String mqttWillTopic = "infob3it/088/" + clientId + "/status";
byte mqttWillQoS = 0;
boolean mqttWillRetain = true;

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
Timer publishModeTimer;

// Globals
Mode mode = automatic;
DisplayState displayState = temperature;
unsigned long readSensorDelay = 1000 * 60 * 1 / 2; // per 1/2 minute(s)
unsigned long rotateStateDelay = 5000;
unsigned long publishModeDelay = 500;
int modeButtonDebounce = 500;
int lightValue;
int moistureValue;
float temperatureValue;
float humidityValue;
float pressureValue;
int moistureThreshold = 0; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
unsigned long lastWaterTime;

void readAllSensorValues()
{
  humidityValue = bme.readHumidity();
  lastWaterTime = wateringMotor.getLastWaterTime();
  lightValue = amux.getLightValue();
  moistureValue = amux.getMoistureValue();
  pressureValue = bme.readPressure() / 100.0F;
  temperatureValue = bme.readTemperature();
}

void publishAllSensorValues(bool isInitPublish = false)
{
  if (isInitPublish)
  {
    Serial.println("initial publish");
    client.publish("infob3it/088/sensor/moisture", "moisture connection setup");
    client.publish("infob3it/088/sensor/pressure", "pressure connection setup");
    client.publish("infob3it/088/sensor/temperature", "temperature connection setup");
    client.publish("infob3it/088/sensor/light", "light connection setup");
    client.publish("infob3it/088/sensor/humidity", "humidity connection setup");
  }
  else
  {
    Serial.println("publishing all sensor values");
    snprintf(humidityPub, MSG_BUFFER_SIZE, "%s", String(humidityValue, 0).c_str()); // to percent
    client.publish("infob3it/088/sensor/humidity", humidityPub);

    snprintf(lightPub, MSG_BUFFER_SIZE, "%s", String(lightValue / 1024 * 100).c_str()); // to percent
    client.publish("infob3it/088/sensor/light", lightPub);

    snprintf(moisturePub, MSG_BUFFER_SIZE, "%s", String(moistureValue / 1024 * 100).c_str()); // to percent
    client.publish("infob3it/088/sensor/moisture", moisturePub);

    snprintf(pressurePub, MSG_BUFFER_SIZE, "%s", String(pressureValue, 0).c_str()); // floor
    client.publish("infob3it/088/sensor/pressure", pressurePub);

    snprintf(temperaturePub, MSG_BUFFER_SIZE, "%s", String(temperatureValue, 1).c_str()); // to one decimal
    client.publish("infob3it/088/sensor/temperature", temperaturePub);
  }
}

void waterPlantActions()
{
  display.drawString(0, 44, "Watering...");
  display.display();
  wateringMotor.giveWater(120, 10000);
  display.clearLine(44, 20);
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (String(topic).endsWith(String("wateringmotor")))
  {
    // give water if an 1 was received as first character
    if ((char)payload[0] == '1')
    {
      if (mode == manual)
      {
        Serial.println("Give water (manual)");
        waterPlantActions();
      }
    }
  }
  else if (String(topic).endsWith(String("sub/mode")))
  {
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1')
    {
      Serial.println("setting mode to automatic");
      mode = automatic;
    }
    else
    {
      Serial.println("setting mode to manual");
      mode = manual;
    }
  }
  else if (String(topic).endsWith(String("allSensorValues")))
  {
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1')
    {
      Serial.println("refreshing sensorvals");
      readAllSensorValues();
      publishAllSensorValues();
    }
  }
}

void mqttReconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword, mqttWillTopic.c_str(), mqttWillQoS, mqttWillRetain, mqttWillMessage))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      publishAllSensorValues(true); // true, because this is the initial publish for all sensor values
      client.publish("infob3it/088/pub/mode", "mode connection setup");
      // ... and resubscribe
      client.subscribe("infob3it/088/actuator/wateringmotor");
      client.subscribe("infob3it/088/sub/mode");
      client.subscribe("infob3it/088/allSensorValues");
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
  // Wifi wifi("Openhuis-iot", "internetofthings");
  // Wifi wifi("Kouros", "interactietech");

  // MQTT connection
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  // BME init
  Wire.begin(D7, D5);
  unsigned status = bme.begin(0x76);

  // Intial Values
  readAllSensorValues();

  serialPrintTimer.start(1000);
  readSensorTimer.start(readSensorDelay);
  publishModeTimer.start(publishModeDelay);
  rotateStateTimer.start(rotateStateDelay);

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

void loop()
{
  if (serialPrintTimer.hasExpired())
  {
    serialPrintTimer.repeat();

    Serial.println("------------------------");
    Serial.println("");
  }

  if (!client.connected())
  {
    mqttReconnect();
  }
  client.loop();

  if (publishModeTimer.hasExpired())
  {
    publishModeTimer.repeat();

    snprintf(modePub, MSG_BUFFER_SIZE, "%i", mode);
    client.publish("infob3it/088/pub/mode", modePub);
  }

  if (readSensorTimer.hasExpired())
  {
    readSensorTimer.repeat();

    readAllSensorValues();
    publishAllSensorValues();
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

    display.clearLine(0, 40);
  }

  if (mode == automatic)
  {
    if (moistureValue < moistureThreshold)
    {
      waterPlantActions();
    }

    builtInLed.on();

    if (modeButton.getState())
    {
      if (millis() >= modeButton.lastPressed + modeButtonDebounce)
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
      if (millis() >= modeButton.lastPressed + modeButtonDebounce)
      {
        mode = automatic;
        modeButton.lastPressed = millis();
      }
    }
  }

  switch (displayState)
  {
  case temperature:
    display.drawString(0, 0, "Temperature:");
    display.drawString(0, 20, String(temperatureValue, 1) + " *C");
    wateringMotor.setLastWaterTimeInCorrectUnit(); // it is done here, so when in waterTime state, displayed time does not change
    break;
  case waterTime:
    display.drawString(0, 0, wateringMotor.lastWaterTimeInCorrectUnit + " ago");
    break;
  case humidity:
    display.drawString(0, 0, "Humidity: " + String(humidityValue, 0) + "%");
    break;
  case moisture:
    display.drawString(0, 0, "Moisture: " + String(moistureValue / 1024 * 100) + "%");
    break;
  case pressure:
    display.drawString(0, 0, "Pressure:");
    display.drawString(0, 20, String(pressureValue, 0) + " hPa");
    break;
  default:
    display.drawString(0, 0, "Watering system running");
    break;
  }

  display.display();
}
