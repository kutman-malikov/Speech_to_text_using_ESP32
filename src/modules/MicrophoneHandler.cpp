#include "modules/MicrophoneHandler.h"
#include "modules/AudioFilter.h"

MicrophoneHandler::MicrophoneHandler(int sdPin, int wsPin, int sckPin, int buttonPin)
    : _sdPin(sdPin), _wsPin(wsPin), _sckPin(sckPin), _buttonPin(buttonPin) {}

void MicrophoneHandler::begin() {
    pinMode(_buttonPin, INPUT);

    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,                                // частота речи
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,         // INMP441: правый канал
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 10,                                  // стабильные буферы
        .dma_buf_len = 512,
        .use_apll = false,                                    // не менять тон
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = _sckPin,
        .ws_io_num = _wsPin,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = _sdPin
    };

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_set_sample_rates(I2S_NUM_0, 16000);

    Serial.println("[Mic] I2S initialized (RIGHT channel, 16kHz, clean+AGC mode)");
}

void MicrophoneHandler::update() {
    bool buttonState = digitalRead(_buttonPin);
    if (buttonState && !_recording) startRecording();
    if (!buttonState && _recording) stopRecording();

    if (_recording) readAudio();
}

bool MicrophoneHandler::isRecording() { return _recording; }

void MicrophoneHandler::startRecording() {
    _recording = true;
    Serial.println("[Mic] Recording started");
}

void MicrophoneHandler::stopRecording() {
    _recording = false;
    Serial.println("[Mic] Recording stopped");
}

void MicrophoneHandler::readAudio() {
    const int bufferSize = 256;
    int32_t samples[bufferSize];
    size_t bytesRead = 0;

    esp_err_t result = i2s_read(I2S_NUM_0, (void*)samples, sizeof(samples), &bytesRead, 1000);

    if (result != ESP_OK || bytesRead == 0) return;

    int samplesRead = bytesRead / sizeof(int32_t);
    int64_t sum = 0;

    for (int i = 0; i < samplesRead; i++) {
        int16_t clean = AudioFilter::process(samples[i]);
        sum += abs(clean);
        // если требуется запись — передай clean в рекордер
    }

    int avgLevel = (samplesRead > 0) ? sum / samplesRead : 0;
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 300) {
        Serial.printf("[Mic] Level: %d\n", avgLevel);
        lastPrint = millis();
    }
}
