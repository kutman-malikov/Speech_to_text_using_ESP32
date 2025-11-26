#pragma once
#include <Arduino.h>

// Структура одной записи сопоставления
struct LangMap {
    const char* code3;    // 3-буквенный код ElevenLabs (ISO-639-3)
    const char* deepl;    // 2-буквенный код для DeepL (ISO-639-1 / DeepL-style)
};

// Таблица доступна снаружи (если надо дебажить)
extern const LangMap LANG_MAP_TABLE[];
extern const size_t LANG_MAP_TABLE_SIZE;

/**
 * @brief Конвертирует 3-буквенный код ElevenLabs (ISO-639-3)
 *        в 2-буквенный код DeepL (ISO-639-1 / DeepL-style).
 *
 * @param code3  Трёхбуквенный код из ElevenLabs (например "eng", "ita", "tur").
 * @return String 
 *    - Двухбуквенный код DeepL (например "EN", "IT", "TR"), если найден.
 *    - "AUTO" если неизвестный код — DeepL сам определит язык.
 */
String elevenToDeepL(const String& code3);
