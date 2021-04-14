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

// Initialize WiFi connection
Wifi wifi("placeholder", "placeholder");

// MQTT connection
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (16)
char moisturePub[MSG_BUFFER_SIZE];
char pressurePub[MSG_BUFFER_SIZE];
char humidityPub[MSG_BUFFER_SIZE];
char temperaturePub[MSG_BUFFER_SIZE];
char lightPub[MSG_BUFFER_SIZE];
char modePub[MSG_BUFFER_SIZE];
const char *mqttServer = "mqtt.uu.nl";
const int mqttPort = 1883;
const char *mqttUser = "placeholder";
const char *mqttPassword = "placeholder";
const std::string clientId = "plantWateringSystem";
const std::string stdTopicPrefix = "infob3it/placeholder/" + clientId + "/";
const char *mqttWillMessage = "Offline";
const std::string mqttWillTopic = stdTopicPrefix + "status";
byte mqttWillQoS = 0;
boolean mqttWillRetain = true;
boolean mqttCleanSession = true;

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
Amux amux(D4, A0); // moisture & light, but also works when only moisture is hooked up
Adafruit_BME280 bme;

// Actuators
Oled display(0x3c, D7, D5);
WateringMotor wateringMotor(D6);
Led builtInLed(D0);

// Timers
Timer readSensorTimer;
Timer rotateStateTimer;
Timer publishModeTimer;
Timer mqttReconnectTimer;
Timer wifiLoadingTimer;

// Globals
Mode mode = automatic;
DisplayState displayState = temperature;
unsigned long readSensorDelay = 1000 * 60 * 1 / 2; // per 1/2 minute(s)
unsigned long rotateStateDelay = 5000;
unsigned long publishModeDelay = 500;
unsigned long mqttReconnectDelay = 5000;
int modeButtonDebounce = 500;
float lightValue;
float moistureValue;
float temperatureValue;
float humidityValue;
float pressureValue;
int moistureThreshold = 30;
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
    client.publish(mqttWillTopic.c_str(), "Online");
    client.publish((stdTopicPrefix + "sensor/moisture").c_str(), "moisture connection setup");
    client.publish((stdTopicPrefix + "sensor/pressure").c_str(), "pressure connection setup");
    client.publish((stdTopicPrefix + "sensor/temperature").c_str(), "temperature connection setup");
    client.publish((stdTopicPrefix + "sensor/light").c_str(), "light connection setup");
    client.publish((stdTopicPrefix + "sensor/humidity").c_str(), "humidity connection setup");
  }
  else
  {
    Serial.println("publishing all sensor values");
    snprintf(humidityPub, MSG_BUFFER_SIZE, "%s", String(humidityValue, 0).c_str()); // to percent
    client.publish((stdTopicPrefix + "sensor/humidity").c_str(), humidityPub);

    snprintf(lightPub, MSG_BUFFER_SIZE, "%s", String(lightValue).c_str()); // to percent
    client.publish((stdTopicPrefix + "sensor/light").c_str(), lightPub);

    snprintf(moisturePub, MSG_BUFFER_SIZE, "%s", String(moistureValue).c_str()); // to percent
    client.publish((stdTopicPrefix + "sensor/moisture").c_str(), moisturePub);

    snprintf(pressurePub, MSG_BUFFER_SIZE, "%s", String(pressureValue, 0).c_str()); // floor
    client.publish((stdTopicPrefix + "sensor/pressure").c_str(), pressurePub);

    snprintf(temperaturePub, MSG_BUFFER_SIZE, "%s", String(temperatureValue, 1).c_str()); // to one decimal
    client.publish((stdTopicPrefix + "sensor/temperature").c_str(), temperaturePub);
  }
}

void waterPlantActions()
{
  display.drawString(0, 44, "Watering...");
  display.display();
  wateringMotor.giveWater(150, 10000);
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
{ // inspiration taken from:
  // https://github.com/knolleary/pubsubclient/blob/master/examples/mqtt_reconnect_nonblocking/mqtt_reconnect_nonblocking.ino
  if (client.connect(clientId.c_str(), mqttUser, mqttPassword, mqttWillTopic.c_str(), mqttWillQoS, mqttWillRetain, mqttWillMessage, mqttCleanSession))
  {
    Serial.println("connected");
    // Once connected, publish an announcement...
    publishAllSensorValues(true); // true, because this is the initial publish for all sensor values
    client.publish(mqttWillTopic.c_str(), "Online");
    client.publish((stdTopicPrefix + "pub/mode").c_str(), "mode connection setup");
    // ... and resubscribe
    client.subscribe((stdTopicPrefix + "actuator/wateringmotor").c_str());
    client.subscribe((stdTopicPrefix + "sub/mode").c_str());
    client.subscribe((stdTopicPrefix + "allSensorValues").c_str());
  }
  else
  {
    Serial.print("failed to connect to MQTT Broker");
    Serial.print(client.state());
    Serial.println(" trying again in 5 seconds...");
  }
}

void setup()
{
  // Initialize Serial monitor (baud rate)
  Serial.begin(115200);

  // Initialize MQTT connection
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);

  // Initialize BME
  Wire.begin(D7, D5);
  bme.begin(0x76);

  // Get initial Values
  readAllSensorValues();

  // Setting Timers
  readSensorTimer.start(readSensorDelay);
  publishModeTimer.start(publishModeDelay);
  rotateStateTimer.start(rotateStateDelay);
  mqttReconnectTimer.start(mqttReconnectDelay);
  wifiLoadingTimer.start(500);

  // Initialize OLED
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
}

void loop()
{
  wifiLoadingTimer = wifi.printStatus(wifiLoadingTimer);

  if (!client.connected())
  { // client not connected
    if (mqttReconnectTimer.hasExpired())
    {
      mqttReconnectTimer.repeat();

      // Attempt to reconnect
      mqttReconnect();
    }
  }
  else
  { // client connected
    client.loop();
  }

  if (publishModeTimer.hasExpired())
  {
    publishModeTimer.repeat();

    snprintf(modePub, MSG_BUFFER_SIZE, "%i", mode);
    client.publish((stdTopicPrefix + "pub/mode").c_str(), modePub);
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
    display.drawString(0, 0, "Watered:");
    display.drawString(0, 20, wateringMotor.lastWaterTimeInCorrectUnit + " ago");
    break;
  case humidity:
    display.drawString(0, 0, "Humidity: " + String(humidityValue, 0) + "%");
    break;
  case moisture:
    display.drawString(0, 0, "Soil moisture:");
    display.drawString(0, 20, String(moistureValue) + "%");
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
