#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

struct WAVHeader {
    char riff[4] = {'R','I','F','F'};
    uint32_t fileSize;
    char wave[4] = {'W','A','V','E'};
    char fmt[4]  = {'f','m','t',' '};
    uint32_t fmtSize = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels = 1;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign = 2;
    uint16_t bitsPerSample = 16;
    char data[4] = {'d','a','t','a'};
    uint32_t dataSize;
};

class AudioRecorder {
public:
    AudioRecorder(int sampleRate, const char* fileName);
    void begin();
    bool start();
    void stop();
    bool isRecording();

    void writeSample(int16_t s); // новый метод
    void writeWavHeader(File& f, uint32_t dataSize);

private:
    int _sampleRate;
    const char* _fileName;
    File _file;
    bool _recording = false;
    uint32_t _dataBytes = 0;
};

#endif
