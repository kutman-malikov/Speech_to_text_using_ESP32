#include "modules/AudioRecorder.h"

AudioRecorder::AudioRecorder(int sampleRate, const char* fileName)
: _sampleRate(sampleRate), _fileName(fileName) {}

void AudioRecorder::begin() {
    if (!LittleFS.begin(true)) {
        Serial.println("[FS] LittleFS mount failed");
    } else {
        Serial.println("[FS] LittleFS mounted");
    }
}

bool AudioRecorder::start() {
    _file = LittleFS.open(_fileName, FILE_WRITE);
    if (!_file) {
        Serial.println("[Rec] Failed to open file");
        return false;
    }

    // Записываем пустой заголовок WAV (44 байта)
    uint8_t emptyHeader[44] = {0};
    _file.write(emptyHeader, sizeof(emptyHeader));

    _dataBytes = 0;
    _recording = true;
    Serial.println("[Rec] Recording started...");
    return true;
}

void AudioRecorder::stop() {
    if (!_recording) return;
    _recording = false;

    writeWavHeader(_file, _dataBytes);
    _file.close();
    Serial.printf("[Rec] Saved %u bytes to %s\n", _dataBytes, _fileName);
}

bool AudioRecorder::isRecording() { return _recording; }

// === Новый метод: запись одного 16-битного сэмпла с усилением ===
void AudioRecorder::writeSample(int16_t s) {
    if (!_recording) return;

    // +20 dB ≈ ×10 по амплитуде
    constexpr float GAIN = 10.0f;

    // считаем во 32-битном, чтобы избежать переполнения при умножении
    int32_t v = static_cast<int32_t>(s) * GAIN;

    // клиппинг в диапазон int16_t
    if (v > 32767) v = 32767;
    else if (v < -32768) v = -32768;

    int16_t amplified = static_cast<int16_t>(v);

    _file.write((uint8_t*)&amplified, sizeof(int16_t));
    _dataBytes += sizeof(int16_t);
}


// === Заголовок WAV ===
void AudioRecorder::writeWavHeader(File& f, uint32_t dataSize) {
    WAVHeader h;
    h.sampleRate = _sampleRate;
    h.byteRate   = _sampleRate * 2;
    h.dataSize   = dataSize;
    h.fileSize   = dataSize + sizeof(WAVHeader) - 8;

    f.seek(0);
    f.write((uint8_t*)&h, sizeof(WAVHeader));
}
