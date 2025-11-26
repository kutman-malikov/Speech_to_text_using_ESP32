#include "LangCodeMapper.h"

// Таблица сопоставлений между кодами ElevenLabs (ISO-639-3) и DeepL (ISO-639-1)
const LangMap LANG_MAP_TABLE[] = {
    // Базовые/часто используемые
    {"eng", "EN"},
    {"deu", "DE"}, {"ger", "DE"},   // немецкий
    {"fra", "FR"}, {"fre", "FR"},   // французский
    {"ita", "IT"},                  // итальянский
    {"spa", "ES"},                  // испанский
    {"rus", "RU"},                  // русский
    {"tur", "TR"},                  // турецкий
    {"por", "PT"},                  // португальский (общий, позже можно разделить на PT-PT / PT-BR)

    // Языки, поддерживаемые DeepL и встречающиеся как ISO-639-3
    {"ara", "AR"},                  // арабский
    {"bul", "BG"},                  // болгарский
    {"ces", "CS"}, {"cze", "CS"},   // чешский
    {"dan", "DA"},                  // датский
    {"ell", "EL"}, {"gre", "EL"},   // греческий
    {"est", "ET"},                  // эстонский
    {"fin", "FI"},                  // финский
    {"hun", "HU"},                  // венгерский
    {"ind", "ID"},                  // индонезийский
    {"jpn", "JA"},                  // японский
    {"kor", "KO"},                  // корейский
    {"lit", "LT"},                  // литовский
    {"lav", "LV"},                  // латышский
    {"nld", "NL"}, {"dut", "NL"},   // нидерландский
    {"nob", "NB"}, {"nor", "NB"},   // норвежский (букмол)
    {"pol", "PL"},                  // польский
    {"ron", "RO"}, {"rum", "RO"},   // румынский
    {"slk", "SK"}, {"slo", "SK"},   // словацкий
    {"slv", "SL"},                  // словенский
    {"swe", "SV"},                  // шведский
    {"ukr", "UK"},                  // украинский
    {"zho", "ZH"},                  // китайский (общий)
};

// Размер таблицы (кол-во элементов)
const size_t LANG_MAP_TABLE_SIZE = sizeof(LANG_MAP_TABLE) / sizeof(LANG_MAP_TABLE[0]);

String elevenToDeepL(const String& code3) {
    if (code3.length() == 0) {
        return String("");
    }

    // Нормализуем к нижнему регистру, чтобы не зависеть от формата
    String norm = code3;
    norm.toLowerCase();

    // Линейный поиск по таблице (таблица маленькая, этого достаточно)
    for (size_t i = 0; i < LANG_MAP_TABLE_SIZE; ++i) {
        if (norm.equalsIgnoreCase(LANG_MAP_TABLE[i].code3)) {
            return String(LANG_MAP_TABLE[i].deepl);
        }
    }

    // Если код не найден — вернём пустую строку.
    // В этом случае в коде, который вызывает DeepL, просто не указывай source_lang,
    // и DeepL сам будет детектить язык.
    return String("");
}
