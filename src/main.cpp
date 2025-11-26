#include <Arduino.h>
#include <WebServer.h>
#include "modules/MicrophoneHandler.h"
#include "modules/AudioRecorder.h"
#include "modules/DisplayHandler.h"
#include "modules/ElevenLabsSTT.h"
#include "modules/wififunc.h"
#include "modules/translator.h" // ✅ Новый модуль DeepL
#include "secrets.h"
#include "modules/LangCodeMapper.h"

#define SAMPLE_RATE 16000
#define WAV_PATH "/record.wav"

// Пины дисплея
#define TFT_CS 15
#define TFT_DC 2
#define TFT_RST 4

// === Объекты ===
DisplayHandler display(TFT_CS, TFT_DC, TFT_RST);
MicrophoneHandler mic(GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_32, GPIO_NUM_34);
AudioRecorder recorder(SAMPLE_RATE, WAV_PATH);
ElevenLabsSTT stt(ELEVENLABS_API_KEY);
WiFiFunc wifi;
Translator translator(DEEPL_API_KEY); // ✅ DeepL API
WebServer server(80);

// ──────────────────────────────
// Веб-страница

void handleRoot()
{
    String html;
    html.reserve(1024);
    html += F("<!doctype html><html><head><meta charset='utf-8'><title>ESP32 Audio</title></head><body>");
    html += F("<h2>Speech-to-Text Translator</h2>");
    if (LittleFS.exists(WAV_PATH))
    {
        File f = LittleFS.open(WAV_PATH, "r");
        size_t sz = f.size();
        f.close();
        html += F("<p>Последняя запись: <b>");
        html += String(sz);
        html += F("</b> bytes</p>");
        html += F("<audio controls src='");
        html += WAV_PATH;
        html += F("'></audio><br>");
        html += F("<a href='");
        html += WAV_PATH;
        html += F("' download>Скачать record.wav</a>");
    }
    else
    {
        html += F("<p><em>Файл ещё не записан — удерживайте кнопку, чтобы начать запись.</em></p>");
    }
    html += F("<hr><p>IP устройства: <code>");
    html += WiFi.localIP().toString();
    html += F("</code></p></body></html>");
    server.send(200, "text/html; charset=utf-8", html);
}

void handleWav()
{
    if (!LittleFS.exists(WAV_PATH))
    {
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
void setup()
{
    Serial.begin(115200);
    delay(500);
    Serial.println("\n[System] Speech-to-Text Translator (DeepL)");

    // ═══ 1. Инициализация дисплея ═══
    display.begin();
    delay(300);

    // ═══ 2. Wi-Fi ═══
    // ═══ 2. Wi-Fi ═══
    display.showBootStatus("WiFi connecting...");

    // Добавляем 2 сети
    wifi.addNetwork(WIFI_SSID, WIFI_PASSWORD);
    wifi.addNetwork(WIFI_SSID_2, WIFI_PASSWORD_2);

    wifi.connect();          // пробует все сети по очереди
    wifi.startMonitorTask(); // следит и восстанавливает при обрыве

    if (WiFi.status() == WL_CONNECTED)
    {
        display.showBootStatus("WiFi OK");
        Serial.println("[WiFi] Connected to " + WiFi.SSID());
    }
    else
    {
        display.showBootStatus("WiFi FAILED!");
        Serial.println("[WiFi] Initial connection failed. Monitor task will retry.");
    }
    delay(300);

    // ═══ 3. Микрофон ═══
    display.showBootStatus("Init microphone...");
    mic.begin();
    display.showBootStatus("Mic OK");
    delay(300);

    // ═══ 4. Рекордер ═══
    display.showBootStatus("Init recorder...");
    recorder.begin();
    display.showBootStatus("Recorder OK");
    delay(300);

    // ═══ 5. Callback: микрофон → рекордер ═══
    mic._onSample = [&](int16_t sample)
    {
        if (recorder.isRecording())
            recorder.writeSample(sample);
    };

    // ═══ 6. Веб-сервер ═══
    display.showBootStatus("Starting server...");
    server.on("/", handleRoot);
    server.on(WAV_PATH, handleWav);
    server.begin();
    display.showBootStatus("Server OK");
    delay(300);

    display.showBootStatus("System Ready!");
    Serial.println("[System] Ready. Hold button to record.");
    Serial.println("[Web] Open http://" + WiFi.localIP().toString());

    delay(1000);

    // ═══ 7. Главное окно ═══
    display.initMainInterface();
}

// ──────────────────────────────
// Loop
// ──────────────────────────────
void loop()
{
    server.handleClient();
    mic.update();

    static bool prev = false;
    static unsigned long debounceTimer = 0;
    static bool debounceState = false;
    static unsigned long releaseTime = 0;
    static bool postRecord = false;

    bool rawPressed = mic.isRecording();

    // --- антидребезг кнопки ---
    if (rawPressed != debounceState)
    {
        debounceTimer = millis();
        debounceState = rawPressed;
    }
    if (millis() - debounceTimer < 20)
        return;

    // --- начало записи ---
    if (debounceState && !prev && !postRecord)
    {
        recorder.start();
        display.showStatusBar("Recording", ILI9341_RED);
        display.showTranscriptionText("Listening...\nSpeak now.");
        Serial.println("[System] Recording started...");
    }

    // --- отпускание кнопки ---
    if (!debounceState && prev)
    {
        releaseTime = millis();
        postRecord = true;
        Serial.println("[System] Button released, finishing...");
    }

    // --- завершение записи и перевод ---
    if (postRecord && (millis() - releaseTime) > 100)
    {
        recorder.stop();
        postRecord = false;

        display.showStatusBar("Processing...", ILI9341_YELLOW);
        display.showTranscriptionText("Processing audio...\nUploading to ElevenLabs...");
        Serial.println("[System] Recording stopped.");

        // шаг 1: распознавание речи
        String originalText = stt.transcribeFile(WAV_PATH);
        String translatedText = "";
        String langDetected = elevenToDeepL(stt.lastLanguageDetected());

        // шаг 2: перевод через DeepL
        if (originalText.length())
        {
            display.showStatusBar("Translating...", ILI9341_CYAN);
            display.showTranscriptionText("Translating text via DeepL...");
            translatedText = translator.translate(originalText, "EN-US", langDetected);
        }

        // шаг 3: отображение результата
        display.showStatusBar("Ready", ILI9341_GREEN);
        if (originalText.length())
        {
            String combined = "EN:\n" + translatedText;
            display.showTranscriptionText(combined);
            Serial.println("[Result] Original:");
            Serial.println(originalText);
            Serial.println("[Result] Translated:");
            Serial.println(translatedText);
        }
        else
        {
            display.showTranscriptionText("No transcription received.\nPlease try again.");
            Serial.println("[Result] No transcription received.");
        }
    }

    prev = debounceState;
    delay(1);
}
