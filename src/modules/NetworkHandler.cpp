#include "modules/MicrophoneHandler.h"
#include <Arduino.h>

MicrophoneHandler::MicrophoneHandler(int sdPin, int wsPin, int sckPin, int buttonPin)
    : _sdPin(sdPin), _wsPin(wsPin), _sckPin(sckPin), _buttonPin(buttonPin) {}

void MicrophoneHandler::begin() {
    // Настройка кнопки
    pinMode(_buttonPin, INPUT);

    // Настройка I²S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,           // 16 kHz — достаточно для речи
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 256,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = _sckPin,    // SCK
        .ws_io_num = _wsPin,      // WS (LRCLK)
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = _sdPin     // SD
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_zero_dma_buffer(I2S_NUM_0);

    Serial.println("[Mic] I2S initialized");
}

void MicrophoneHandler::update() {
    bool buttonState = digitalRead(_buttonPin);

    if (buttonState && !_recording) startRecording();
    if (!buttonState && _recording) stopRecording();

    if (_recording) readAudio();
}

bool MicrophoneHandler::isRecording() {
    return _recording;
}

void MicrophoneHandler::startRecording() {
    _recording = true;
    Serial.println("[Mic] Recording started");
}

void MicrophoneHandler::stopRecording() {
    _recording = false;
    Serial.println("[Mic] Recording stopped");
}

void MicrophoneHandler::readAudio() {
    const int bufferSize = 1024;
    int32_t samples[bufferSize];
    size_t bytesRead = 0;

    esp_err_t result = i2s_read(I2S_NUM_0, (void*)samples, sizeof(samples), &bytesRead, 1000);

    if (result != ESP_OK) {
        Serial.printf("[Mic] i2s_read error: %d\n", result);
        return;
    }

    if (bytesRead == 0) {
        Serial.println("[Mic] No data read!");
        return;
    }

    int64_t sum = 0;
    int samplesRead = bytesRead / 4;
    for (int i = 0; i < samplesRead; i++) {
        sum += abs(samples[i] >> 14); // уменьшение 32-битных данных
    }
    int avgLevel = sum / samplesRead;

    Serial.printf("[Mic] Level: %d\n", avgLevel);
}

