#ifndef ELEVENLABS_STT_H
#define ELEVENLABS_STT_H

#include <Arduino.h>
#include <WiFi.h>

class ElevenLabsSTT
{
public:
    explicit ElevenLabsSTT(const char *apiKey);
    String transcribeFile(const char *path);
    String lastLanguageDetected();

private:
    const char *_apiKey;
    const char *_url;
};

#endif
