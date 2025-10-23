#include <Arduino.h>
#include "modules/MicrophoneHandler.h"
#include "modules/AudioRecorder.h"
#include "modules/ElevenLabsSTT.h"
#include "modules/wififunc.h"   // ✅ добавляем WiFi модуль

#define SAMPLE_RATE 16000
#define API_KEY "YOUR_ELEVENLABS_API_KEY"

// === Объекты ===
MicrophoneHandler mic(GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_32, GPIO_NUM_34);
AudioRecorder recorder(SAMPLE_RATE, "/record.wav");
ElevenLabsSTT stt(API_KEY);
WiFiFunc wifi;  // ✅ объект Wi-Fi

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("[System] Speech-to-Text test (ElevenLabs)");

    // --- Подключаемся к Wi-Fi ---
    wifi.addNetwork("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD"); // 🔧 Введи свои данные
    wifi.connect();
    wifi.startMonitorTask();

    // --- Модули ---
    mic.begin();
    recorder.begin();

    Serial.println("[System] Ready. Hold button to record.");
}

void loop() {
    mic.update();
    bool pressed = mic.isRecording();
    static bool prev = false;

    // --- Управление записью ---
    if (pressed && !prev) {
        recorder.start();
    }
    if (!pressed && prev) {
        recorder.stop();
        Serial.println("[System] Uploading to ElevenLabs...");
        String text = stt.transcribeFile("/record.wav");
        if (text.length()) {
            Serial.println("[Result] Transcription:");
            Serial.println(text);
        } else {
            Serial.println("[Result] No transcription received.");
        }
    }
    prev = pressed;

    // --- Запись аудио ---
    if (recorder.isRecording()) {
        const int BUF = 256;
        int32_t buf[BUF];
        size_t bytesRead = 0;

        if (i2s_read(I2S_NUM_0, buf, sizeof(buf), &bytesRead, 1000) == ESP_OK && bytesRead > 0)
            recorder.writeSamples(buf, bytesRead / sizeof(int32_t));
    }

    delay(1);
}
