#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

// Определение пинов
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4
// MOSI = 23, SCK = 18, MISO = 19 (аппаратный SPI)

// Внешнее объявление (определяется в main.cpp)
extern Adafruit_ILI9341 tft;

// Функции для работы с дисплеем
void initDisplay();
void testDisplay();

#endif