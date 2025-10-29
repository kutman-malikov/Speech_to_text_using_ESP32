#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <Arduino.h>
#include <HTTPClient.h>

class Translator {
public:
    Translator(const char* apiKey);

    // targetLang: "EN", "FR", "TR", ...
    // sourceLang: "AUTO" для автоопределения
    String translate(const String& text,
                     const String& targetLang,
                     const String& sourceLang = "AUTO");

private:
    const char* _apiKey;
};

#endif
