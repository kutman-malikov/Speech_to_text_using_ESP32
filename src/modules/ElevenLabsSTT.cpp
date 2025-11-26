#include "modules/ElevenLabsSTT.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <LittleFS.h>

String langCode = "auto";

ElevenLabsSTT::ElevenLabsSTT(const char *apiKey)
    : _apiKey(apiKey),
      _url("https://api.elevenlabs.io/v1/speech-to-text") {}

// ─────────────────────────────────────────────────────────────
// Основная функция распознавания речи
// ─────────────────────────────────────────────────────────────
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

    if (fileSize == 0)
    {
        Serial.println("[STT] File is empty");
        file.close();
        return "";
    }

    if (fileSize > 70000)
    {
        Serial.println("[STT] File too large (>70KB)");
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

    // === Формируем multipart тело запроса ===
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
        Serial.println("[STT] Request failed:");
        Serial.println(response);
        return "";
    }

    // === Парсинг JSON ответа ===
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
        Serial.println(response);
        return "";
    }

    String text = doc["text"].as<String>();

    if (doc.containsKey("language_code"))
    {
        langCode = doc["language_code"].as<String>();
        Serial.printf("[STT] Detected language: %s\n", langCode.c_str());
    }

    Serial.printf("[STT] Original text: %s\n", text.c_str());

    return text;
}
String ElevenLabsSTT::lastLanguageDetected() {
    return langCode;
}
