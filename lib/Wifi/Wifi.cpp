#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Wifi.h"
#include "../Timer/Timer.h"

Wifi::Wifi(char *ssid, char *password)
{
    Timer loadingTimer;
    loadingTimer.start(500);

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.print(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        if (loadingTimer.hasExpired())
        {
            loadingTimer.repeat();
            Serial.print(".");
            yield();
        }
    }

    Serial.println("");
    Serial.print("WiFi connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}