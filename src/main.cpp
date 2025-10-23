#include <Arduino.h>
#include <WebServer.h>
#include "modules/MicrophoneHandler.h"
#include "modules/AudioRecorder.h"
#include "modules/ElevenLabsSTT.h"
#include "modules/wififunc.h"

#define SAMPLE_RATE 16000
#define API_KEY "sk_5f808c3a868f9f838539c65ec4fb3604817a3ae04ee46463"

MicrophoneHandler mic(GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_32, GPIO_NUM_34);
AudioRecorder recorder(SAMPLE_RATE, "/record.wav");
ElevenLabsSTT stt(API_KEY);
WiFiFunc wifi;
WebServer server(80);

// ──────────────────────────────
// простая веб-страница
// ──────────────────────────────
void handleRoot() {
    String html;
    html.reserve(1024);
    html += F("<!doctype html><html><head><meta charset='utf-8'><title>ESP32 Audio</title></head><body>");
    html += F("<h2>Speech-to-Text Recorder</h2>");
    if (LittleFS.exists("/record.wav")) {
        File f = LittleFS.open("/record.wav", "r");
        size_t sz = f.size();
        f.close();
        html += F("<p>Последняя запись: <b>");
        html += String(sz);
        html += F("</b> bytes</p>");
        html += F("<audio controls src='/record.wav'></audio><br>");
        html += F("<a href='/record.wav' download>Скачать record.wav</a>");
    } else {
        html += F("<p><em>Файл ещё не записан — удерживайте кнопку, чтобы начать запись.</em></p>");
    }
    html += F("<hr><p>IP устройства: <code>");
    html += WiFi.localIP().toString();
    html += F("</code></p></body></html>");
    server.send(200, "text/html; charset=utf-8", html);
}

void handleWav() {
    if (!LittleFS.exists("/record.wav")) {
        server.send(404, "text/plain", "record.wav not found");
        return;
    }
    File f = LittleFS.open("/record.wav", "r");
    server.streamFile(f, "audio/wav");
    f.close();
}

// ──────────────────────────────
// setup
// ──────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("[System] Speech-to-Text with Web Access");

    // Wi-Fi
    wifi.addNetwork("astronet", "20052007.");
    wifi.connect();
    wifi.startMonitorTask();

    // запуск модулей
    mic.begin();
    recorder.begin();

    // WebServer
    server.on("/", handleRoot);
    server.on("/record.wav", handleWav);
    server.begin();

    Serial.println("[System] Ready. Hold button to record.");
    Serial.println("[Web] Open http://" + WiFi.localIP().toString());
}

// ──────────────────────────────
// loop
// ──────────────────────────────
void loop() {
    server.handleClient();

    mic.update();
    bool pressed = mic.isRecording();
    static bool prev = false;

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

    if (recorder.isRecording()) {
        const int BUF = 256;
        int32_t buf[BUF];
        size_t bytesRead = 0;
        if (i2s_read(I2S_NUM_0, buf, sizeof(buf), &bytesRead, 1000) == ESP_OK && bytesRead > 0)
            recorder.writeSamples(buf, bytesRead / sizeof(int32_t));
    }

    delay(1);
}
