#include "LangCodeMapper.h"

// Таблица сопоставлений между ISO-639-3 (ElevenLabs) и ISO-639-1 (DeepL)
const LangMap LANG_MAP_TABLE[] = {
    {"eng", "EN"},
    {"deu", "DE"}, {"ger", "DE"},
    {"fra", "FR"}, {"fre", "FR"},
    {"ita", "IT"},
    {"spa", "ES"},
    {"rus", "RU"},
    {"tur", "TR"},
    {"por", "PT"},

    {"ara", "AR"},
    {"bul", "BG"},
    {"ces", "CS"}, {"cze", "CS"},
    {"dan", "DA"},
    {"ell", "EL"}, {"gre", "EL"},
    {"est", "ET"},
    {"fin", "FI"},
    {"hun", "HU"},
    {"ind", "ID"},
    {"jpn", "JA"},
    {"kor", "KO"},
    {"lit", "LT"},
    {"lav", "LV"},
    {"nld", "NL"}, {"dut", "NL"},
    {"nob", "NB"}, {"nor", "NB"},
    {"pol", "PL"},
    {"ron", "RO"}, {"rum", "RO"},
    {"slk", "SK"}, {"slo", "SK"},
    {"slv", "SL"},
    {"swe", "SV"},
    {"ukr", "UK"},
    {"zho", "ZH"},
};

const size_t LANG_MAP_TABLE_SIZE =
    sizeof(LANG_MAP_TABLE) / sizeof(LANG_MAP_TABLE[0]);

String elevenToDeepL(const String& code3) {
    if (code3.length() == 0) {
        return "AUTO";  // нет языка → DeepL сам разберётся
    }

    // нормализуем в lowercase
    String norm = code3;
    norm.toLowerCase();

    // ищем в таблице
    for (size_t i = 0; i < LANG_MAP_TABLE_SIZE; ++i) {
        if (norm.equalsIgnoreCase(LANG_MAP_TABLE[i].code3)) {
            return String(LANG_MAP_TABLE[i].deepl);
        }
    }

    // если код неизвестен → AUTO
    return "AUTO";
}
