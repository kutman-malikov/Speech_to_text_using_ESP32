#include "modules/ElevenLabsSTT.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>

ElevenLabsSTT::ElevenLabsSTT(const char *apiKey) : _apiKey(apiKey) {}

// Функция перевода через LibreTranslate (бесплатный API)
String translateToEnglish(const String& text, const String& sourceLang) {
    if (text.length() == 0) return "";
    
    // Если текст уже на английском (латиница), не переводим
    bool isLatin = true;
    for (unsigned int i = 0; i < text.length(); i++) {
        if ((unsigned char)text[i] > 127) {
            isLatin = false;
            break;
        }
    }
    if (isLatin) {
        Serial.println("[Translate] Text is already in Latin, skipping");
        return text;
    }
    
    HTTPClient http;
    
    // Используем публичный LibreTranslate сервер
    String url = "https://libretranslate.com/translate";
    
    Serial.println("[Translate] Translating to English via LibreTranslate...");
    
    if (!http.begin(url)) {
        Serial.println("[Translate] Failed to connect");
        return text;
    }
    
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(15000);
    
    // Преобразуем язык "rus" -> "ru"
    String lang = sourceLang;
    if (lang == "rus") lang = "ru";
    else if (lang == "eng") lang = "en";
    else if (lang.length() > 2) lang = lang.substring(0, 2);
    
    // Создаем JSON тело запроса
    DynamicJsonDocument requestDoc(2048);
    requestDoc["q"] = text;
    requestDoc["source"] = lang;
    requestDoc["target"] = "en";
    requestDoc["format"] = "text";
    
    String requestBody;
    serializeJson(requestDoc, requestBody);
    
    Serial.printf("[Translate] Request: %s\n", requestBody.c_str());
    
    int code = http.POST(requestBody);
    String response = http.getString();
    http.end();
    
    Serial.printf("[Translate] HTTP code: %d\n", code);
    Serial.printf("[Translate] Response: %s\n", response.c_str());
    
    if (code != 200) {
        Serial.printf("[Translate] HTTP error: %d\n", code);
        return text;
    }
    
    // Парсинг ответа
    DynamicJsonDocument responseDoc(2048);
    DeserializationError error = deserializeJson(responseDoc, response);
    
    if (error) {
        Serial.printf("[Translate] JSON parse error: %s\n", error.c_str());
        return text;
    }
    
    if (!responseDoc.containsKey("translatedText")) {
        Serial.println("[Translate] No translatedText in response");
        return text;
    }
    
    String translated = responseDoc["translatedText"].as<String>();
    Serial.printf("[Translate] SUCCESS! Translated: %s\n", translated.c_str());
    
    return translated;
}

String ElevenLabsSTT::transcribeFile(const char *path)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("[STT] WiFi not connected");
        return "";
    }

    File file = LittleFS.open(path, "r");
    if (!file)
    {
        Serial.printf("[STT] Cannot open %s\n", path);
        return "";
    }

    size_t fileSize = file.size();
    Serial.printf("[STT] File size: %d bytes\n", fileSize);
    
    if (fileSize > 500000)
    {
        Serial.println("[STT] File too large (>500KB)");
        file.close();
        return "";
    }

    uint8_t *data = (uint8_t *)malloc(fileSize);
    if (!data)
    {
        Serial.println("[STT] Memory allocation failed");
        file.close();
        return "";
    }
    file.read(data, fileSize);
    file.close();

    HTTPClient http;
    if (!http.begin(_url))
    {
        Serial.println("[STT] Failed to connect to ElevenLabs API");
        free(data);
        return "";
    }

    http.setTimeout(30000);
    http.setConnectTimeout(10000);
    http.addHeader("xi-api-key", _apiKey);

    String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    String bodyStart =
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"model_id\"\r\n\r\n"
        "scribe_v1\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"language\"\r\n\r\n"
        "auto\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n"
        "Content-Type: audio/wav\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";
    size_t totalSize = bodyStart.length() + fileSize + bodyEnd.length();

    uint8_t *body = (uint8_t *)malloc(totalSize);
    if (!body)
    {
        Serial.println("[STT] Failed to allocate request body");
        free(data);
        http.end();
        return "";
    }
    
    memcpy(body, bodyStart.c_str(), bodyStart.length());
    memcpy(body + bodyStart.length(), data, fileSize);
    memcpy(body + bodyStart.length() + fileSize, bodyEnd.c_str(), bodyEnd.length());
    free(data);

    Serial.println("[STT] Sending to ElevenLabs...");
    int code = http.POST(body, totalSize);
    free(body);

    String response = http.getString();
    http.end();

    Serial.printf("[STT] HTTP %d\n", code);

    if (code != 200)
    {
        Serial.printf("[STT] HTTP error: %d\n", code);
        return "";
    }

    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error)
    {
        Serial.printf("[STT] JSON parse failed: %s\n", error.c_str());
        return "";
    }

    if (!doc.containsKey("text"))
    {
        Serial.println("[STT] No 'text' field in response");
        return "";
    }

    String text = doc["text"].as<String>();
    String langCode = "auto";
    
    if (doc.containsKey("language_code")) {
        langCode = doc["language_code"].as<String>();
        Serial.printf("[STT] Detected language: %s\n", langCode.c_str());
    }
    
    Serial.printf("[STT] Original text: %s\n", text.c_str());
    
    // Переводим на английский
    String translatedText = translateToEnglish(text, langCode);
    
    return translatedText;
}