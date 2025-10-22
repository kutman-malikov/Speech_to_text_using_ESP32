#pragma once
#include <driver/i2s.h>

// === Пины INMP441 ===
// Подключения у тебя: SD=33, WS=25, SCK=32, L/R=GND
#define I2S_SD   GPIO_NUM_33   // SD (data)
#define I2S_WS   GPIO_NUM_25   // WS / LRCL
#define I2S_SCK  GPIO_NUM_32   // SCK / BCLK

// Кнопка: «зажата — пишем», «отпустил — стоп»
#define BUTTON_PIN GPIO_NUM_34

// Настройки I2S
#define I2S_PORT     I2S_NUM_0
#define SAMPLE_RATE  16000
