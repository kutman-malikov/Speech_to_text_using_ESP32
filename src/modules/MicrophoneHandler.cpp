#include "modules/MicrophoneHandler.h"
#include <driver/i2s.h>

MicrophoneHandler::MicrophoneHandler(int sdPin, int wsPin, int sckPin, int buttonPin)
    : _sdPin(sdPin), _wsPin(wsPin), _sckPin(sckPin), _buttonPin(buttonPin) {}

void MicrophoneHandler::begin() {
    pinMode(_buttonPin, INPUT);

    // --- Конфигурация I2S ---
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,  // стабильная частота (16kHz)
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,  // INMP441: правый канал
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 512,
        .use_apll = false, // ⚠️ Без APLL — устраняет искажения частоты
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = _sckPin,
        .ws_io_num = _wsPin,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = _sdPin
    };

    // --- Установка драйвера ---
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("[Mic] Ошибка установки драйвера I2S: %d\n", err);
        return;
    }

    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("[Mic] Ошибка установки пинов I2S: %d\n", err);
        return;
    }

    // --- Критически важно: очистка DMA ---
    i2s_zero_dma_buffer(I2S_NUM_0);

    Serial.println("[Mic] I2S инициализирован (16kHz, 32-bit, RIGHT channel, NO FILTER)");
}

void MicrophoneHandler::update() {
    bool pressed = digitalRead(_buttonPin);
    if (pressed && !_recording) startRecording();
    if (!pressed && _recording) stopRecording();

    if (_recording) readAudio();
}

bool MicrophoneHandler::isRecording() { return _recording; }

void MicrophoneHandler::startRecording() {
    _recording = true;
    Serial.println("[Mic] Recording started (RAW)");
}

void MicrophoneHandler::stopRecording() {
    _recording = false;
    Serial.println("[Mic] Recording stopped (RAW)");
}

// --- Чтение микрофона ---
void MicrophoneHandler::readAudio() {
    const int BUF = 512;
    int32_t samples[BUF];
    size_t bytesRead = 0;

    esp_err_t result = i2s_read(I2S_NUM_0, (void*)samples, sizeof(samples), &bytesRead, portMAX_DELAY);
    if (result != ESP_OK || bytesRead == 0) return;

    int samplesRead = bytesRead / sizeof(int32_t);
    int64_t avgLevel = 0;

    for (int i = 0; i < samplesRead; i++) {
        // Правильное преобразование 24→16 бит, без искажений
        int16_t s16 = (int16_t)(samples[i] >> 16);
        avgLevel += abs(s16);

        // если используется AudioRecorder:
        if (_onSample) _onSample(s16);
    }

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 300) {
        Serial.printf("[Mic RAW] Level: %lld\n", avgLevel / samplesRead);
        lastPrint = millis();
    }
}
