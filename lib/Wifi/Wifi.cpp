#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Wifi.h"

Wifi::Wifi(char *ssid, char *password)
{
    this->ssid = ssid;
    this->justConnected = false;

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.print(ssid);

    // WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}

bool Wifi::isConnected()
{
    this->justConnected = WiFi.status() == WL_CONNECTED;

    return this->justConnected;
}

Timer Wifi::printStatus(Timer loadingTimer)
{ // non-blocking wifi connection status feedback:)!!!

    if (!this->justConnected && this->isConnected())
    {
        Serial.println("-----------------------------");
        Serial.print("WiFi connected to ");
        Serial.println(this->ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.println("-----------------------------");
    }
    else if (!this->isConnected())
    {
        if (loadingTimer.hasExpired())
        {
            loadingTimer.repeat();
            Serial.print(".");
        }
    }

    return loadingTimer;
}