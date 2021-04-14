#include <Arduino.h>
#include "../Timer/Timer.h"

#ifndef INFOB3IT_WIFI_H
#define INFOB3IT_WIFI_H

class Wifi
{

private:
    char *ssid;
    bool justConnected;

public:
    // Constructor
    Wifi(char *ssid, char *password);
    // Methods
    bool isConnected();
    Timer printStatus(Timer loadingTimer);
};

#endif //INFOB3IT_WIFI_H