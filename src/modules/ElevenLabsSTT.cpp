#include "modules/ElevenLabsSTT.h"
#include <ArduinoJson.h>

ElevenLabsSTT::ElevenLabsSTT(const char *apiKey) : _apiKey(apiKey) {}

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
                          "--" +
        boundary + "\r\n"
                   "Content-Disposition: form-data; name=\"languages\"\r\n\r\n"
                   "ru,tr,ar,zh,fr,en,ja\r\n" // ✅ ограничение 7 языков
                   "--" +
        boundary + "\r\n"
                   "Content-Disposition: form-data; name=\"file\"; filename=\"audio.wav\"\r\n"
                   "Content-Type: audio/wav\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";
    size_t totalSize = bodyStart.length() + fileSize + bodyEnd.length();

    uint8_t *body = (uint8_t *)malloc(totalSize);
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
    Serial.println("[STT] Response:");
    Serial.println(response);

    if (code != 200)
        return "";

    DynamicJsonDocument doc(2048);
    if (deserializeJson(doc, response) != DeserializationError::Ok)
    {
        Serial.println("[STT] JSON parse failed");
        return "";
    }

    if (!doc.containsKey("text"))
    {
        Serial.println("[STT] No 'text' field in response");
        return "";
    }

    String text = doc["text"].as<String>();
    Serial.printf("[STT] Recognized text: %s\n", text.c_str());
    return text;
}
