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

void AudioRecorder::writeSamples(int32_t* samples, int samplesCount) {
    if (!_recording) return;

    static int16_t buf16[256];
    int processed = 0;
    while (processed < samplesCount) {
        int chunk = min(samplesCount - processed, 256);
        for (int i = 0; i < chunk; i++) {
            buf16[i] = (int16_t)(samples[processed + i] >> 11); // под правый канал
        }
        _file.write((uint8_t*)buf16, chunk * sizeof(int16_t));
        _dataBytes += chunk * 2;
        processed += chunk;
    }
}

void AudioRecorder::writeWavHeader(File& f, uint32_t dataSize) {
    WAVHeader h;
    h.sampleRate = _sampleRate;
    h.byteRate   = _sampleRate * 2;
    h.dataSize   = dataSize;
    h.fileSize   = dataSize + sizeof(WAVHeader) - 8;
    f.seek(0);
    f.write((uint8_t*)&h, sizeof(WAVHeader));
}
