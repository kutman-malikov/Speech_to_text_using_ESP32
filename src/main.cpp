#include <Arduino.h>
#include <WebServer.h>
#include "modules/MicrophoneHandler.h"
#include "modules/AudioRecorder.h"
#include "modules/DisplayHandler.h"  // ← ДОБАВЛЕНО
#include "modules/ElevenLabsSTT.h"
#include "modules/wififunc.h"
#include "secrets.h"

#define SAMPLE_RATE 16000
#define WAV_PATH "/record.wav"

// Пины для дисплея (аппаратный SPI: MOSI=23, SCK=18, MISO=19)
#define TFT_CS   15  // ← ДОБАВЛЕНО
#define TFT_DC   2   // ← ДОБАВЛЕНО
#define TFT_RST  4   // ← ДОБАВЛЕНО

// === Объекты ===
DisplayHandler display(TFT_CS, TFT_DC, TFT_RST);  // ← ДОБАВЛЕНО
MicrophoneHandler mic(GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_32, GPIO_NUM_34);
AudioRecorder recorder(SAMPLE_RATE, WAV_PATH);
ElevenLabsSTT stt(ELEVENLABS_API_KEY);
WiFiFunc wifi;
WebServer server(80);

// ──────────────────────────────
// Веб-страница
// ──────────────────────────────
void handleRoot() {
    String html;
    html.reserve(1024);
    html += F("<!doctype html><html><head><meta charset='utf-8'><title>ESP32 Audio</title></head><body>");
    html += F("<h2>Speech-to-Text Recorder</h2>");
    if (LittleFS.exists(WAV_PATH)) {
        File f = LittleFS.open(WAV_PATH, "r");
        size_t sz = f.size(); f.close();
        html += F("<p>Последняя запись: <b>");
        html += String(sz);
        html += F("</b> bytes</p>");
        html += F("<audio controls src='");
        html += WAV_PATH;
        html += F("'></audio><br>");
        html += F("<a href='");
        html += WAV_PATH;
        html += F("' download>Скачать record.wav</a>");
    } else {
        html += F("<p><em>Файл ещё не записан — удерживайте кнопку, чтобы начать запись.</em></p>");
    }
    html += F("<hr><p>IP устройства: <code>");
    html += WiFi.localIP().toString();
    html += F("</code></p></body></html>");
    server.send(200, "text/html; charset=utf-8", html);
}

void handleWav() {
    if (!LittleFS.exists(WAV_PATH)) {
        server.send(404, "text/plain", "record.wav not found");
        return;
    }
    File f = LittleFS.open(WAV_PATH, "r");
    server.streamFile(f, "audio/wav");
    f.close();
}

// ──────────────────────────────
// Setup
// ──────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n[System] Speech-to-Text with Display");

    // ═══ 1. Инициализация дисплея ═══
    display.begin();  // ← ДОБАВЛЕНО
    delay(300);

    // ═══ 2. Wi-Fi ═══
    display.showBootStatus("WiFi connecting...");  // ← ДОБАВЛЕНО
    wifi.addNetwork(WIFI_SSID, WIFI_PASSWORD);
    wifi.connect();
    wifi.startMonitorTask();
    
    if (WiFi.status() == WL_CONNECTED) {
        display.showBootStatus("WiFi OK");  // ← ДОБАВЛЕНО
    } else {
        display.showBootStatus("WiFi FAILED!");  // ← ДОБАВЛЕНО
    }
    delay(300);

    // ═══ 3. Микрофон ═══
    display.showBootStatus("Init microphone...");  // ← ДОБАВЛЕНО
    mic.begin();
    display.showBootStatus("Mic OK");  // ← ДОБАВЛЕНО
    delay(300);

    // ═══ 4. Рекордер ═══
    display.showBootStatus("Init recorder...");  // ← ДОБАВЛЕНО
    recorder.begin();
    display.showBootStatus("Recorder OK");  // ← ДОБАВЛЕНО
    delay(300);

    // ═══ 5. Callback: микрофон → рекордер ═══
    mic._onSample = [&](int16_t sample) {
        if (recorder.isRecording()) recorder.writeSample(sample);
    };

    // ═══ 6. Веб-сервер ═══
    display.showBootStatus("Starting server...");  // ← ДОБАВЛЕНО
    server.on("/", handleRoot);
    server.on(WAV_PATH, handleWav);
    server.begin();
    display.showBootStatus("Server OK");  // ← ДОБАВЛЕНО
    delay(300);

    display.showBootStatus("System Ready!");  // ← ДОБАВЛЕНО
    Serial.println("[System] Ready. Hold button to record.");
    Serial.println("[Web] Open http://" + WiFi.localIP().toString());
    
    delay(1500);
    
    // ═══ 7. Переход в главный интерфейс ═══
    display.initMainInterface();  // ← ДОБАВЛЕНО
}

// ──────────────────────────────
// Loop
// ──────────────────────────────
void loop() {
    server.handleClient();
    mic.update();

    static bool prev = false;
    static unsigned long debounceTimer = 0;
    static bool debounceState = false;
    static unsigned long releaseTime = 0;
    static bool postRecord = false;

    bool rawPressed = mic.isRecording();

    // --- антидребезг кнопки ---
    if (rawPressed != debounceState) {
        debounceTimer = millis();
        debounceState = rawPressed;
    }
    if (millis() - debounceTimer < 20) return;

    // --- начало записи ---
    if (debounceState && !prev && !postRecord) {
        recorder.start();
        display.showStatusBar("Recording", ILI9341_RED);  // ← ДОБАВЛЕНО
        display.showTranscriptionText("Listening...\nSpeak now.");  // ← ДОБАВЛЕНО
        Serial.println("[System] Recording started...");
    }

    // --- отпускание кнопки ---
    if (!debounceState && prev) {
        releaseTime = millis();
        postRecord = true;
        Serial.println("[System] Button released, finishing...");
    }

    // --- завершение ---
    if (postRecord && (millis() - releaseTime) > 100) {
        recorder.stop();
        postRecord = false;
        
        display.showStatusBar("Processing...", ILI9341_YELLOW);  // ← ДОБАВЛЕНО
        display.showTranscriptionText("Processing audio...\nUploading to ElevenLabs...");  // ← ДОБАВЛЕНО
        Serial.println("[System] Recording stopped.");

        Serial.println("[System] Uploading to ElevenLabs...");
        String text = stt.transcribeFile(WAV_PATH);
        
        display.showStatusBar("Ready", ILI9341_GREEN);  // ← ДОБАВЛЕНО
        
        if (text.length()) {
            display.showTranscriptionText(text);  // ← ДОБАВЛЕНО
            Serial.println("[Result] Transcription:");
            Serial.println(text);
        } else {
            display.showTranscriptionText("No transcription received.\nPlease try again.");  // ← ДОБАВЛЕНО
            Serial.println("[Result] No transcription received.");
        }
    }

    prev = debounceState;
    delay(1);
}