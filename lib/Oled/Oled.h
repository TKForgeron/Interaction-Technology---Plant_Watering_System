#include <Arduino.h>
#include "SSD1306Wire.h"


#ifndef INFOB3IT_OLED_H
#define INFOB3IT_OLED_H

class Oled : public SSD1306Wire { 

public:
    // Constructor
    Oled(int address, int sda, int scl);
    // Methods
    void clearLine(int y, int lineHeight);
};

#endif //INFOB3IT_OLED_H