#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

class DisplayHandler {
public:
    DisplayHandler(int8_t cs, int8_t dc, int8_t rst);

    void begin();
    
    // Загрузочное окно
    void showBootStatus(const String& msg);
    
    // Главный интерфейс
    void showStatusBar(const String& state, uint16_t color);
    void showTranscriptionText(const String& text);
    void initMainInterface();
    
    // Утилиты
    void clearScreen();

private:
    Adafruit_ILI9341 tft;
    int8_t _cs, _dc, _rst;
    int _bootLineY;
};

#endif