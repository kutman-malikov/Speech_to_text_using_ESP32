#ifndef ELEVENLABS_STT_H
#define ELEVENLABS_STT_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <LittleFS.h>

class ElevenLabsSTT {
public:
    ElevenLabsSTT(const char* apiKey);
    String transcribeFile(const char* path);

private:
    String _apiKey;
    const char* _url = "https://api.elevenlabs.io/v1/speech-to-text";
};

#endif
