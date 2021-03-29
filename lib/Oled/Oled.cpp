#include <Arduino.h>
#include "Oled.h"

Oled::Oled(int address, int sda, int scl) : SSD1306Wire(address, sda, scl) {
}

void Oled::clearLine(int y,int lineHeight) {
    this->setColor(BLACK);
    this->fillRect(0, y, 128, lineHeight);
    this->display();
    this->setColor(WHITE);
}
