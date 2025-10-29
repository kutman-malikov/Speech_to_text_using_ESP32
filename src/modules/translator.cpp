#include "modules/Translator.h"
#include <ArduinoJson.h>

Translator::Translator(const char* apiKey)
: _apiKey(apiKey) {}

String Translator::translate(const String& text,
                              const String& targetLang,
                              const String& sourceLang) {
    if (text.length() == 0) return "";

    HTTPClient http;
    http.begin("https://api-free.deepl.com/v2/translate");   // ✅ DeepL endpoint
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", String("DeepL-Auth-Key ") + _apiKey);

    // Тело запроса
    String body = "text=" + text + "&target_lang=" + targetLang;
    if (sourceLang != "AUTO") {
        body += "&source_lang=" + sourceLang;
    }

    int httpCode = http.POST(body);
    String translation = "";

    if (httpCode == 200) {
        String response = http.getString();
        Serial.println("[DeepL] Response:");
        Serial.println(response);

        DynamicJsonDocument doc(2048);
        DeserializationError err = deserializeJson(doc, response);
        if (!err && doc["translations"][0]["text"].is<String>()) {
            translation = doc["translations"][0]["text"].as<String>();
        } else {
            Serial.println("[DeepL] JSON parse error");
        }
    } else {
        Serial.printf("[DeepL] HTTP error: %d\n", httpCode);
        Serial.println(http.getString());
    }

    http.end();
    return translation;
}
