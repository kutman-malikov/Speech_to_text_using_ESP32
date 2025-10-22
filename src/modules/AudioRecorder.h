#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <driver/i2s.h>

class AudioRecorder {
public:
    AudioRecorder(int sampleRate, const char* fileName);
    void begin();
    bool start();
    void stop();
    void writeSamples(int32_t* samples, int samplesCount);
    bool isRecording();

private:
    struct WAVHeader {
        char riff[4] = {'R','I','F','F'};
        uint32_t fileSize;
        char wave[4] = {'W','A','V','E'};
        char fmtChunk[4] = {'f','m','t',' '};
        uint32_t fmtChunkSize = 16;
        uint16_t audioFormat = 1;
        uint16_t numChannels = 1;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign = 2;
        uint16_t bitsPerSample = 16;
        char dataChunk[4] = {'d','a','t','a'};
        uint32_t dataSize;
    };

    File _file;
    const char* _fileName;
    int _sampleRate;
    bool _recording = false;
    uint32_t _dataBytes = 0;

    void writeWavHeader(File& file, uint32_t dataSize);
};

#endif
