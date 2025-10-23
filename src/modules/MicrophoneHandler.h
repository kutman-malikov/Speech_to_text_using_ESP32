#ifndef MICROPHONE_HANDLER_H
#define MICROPHONE_HANDLER_H

#include <Arduino.h>
#include <driver/i2s.h>

class MicrophoneHandler {
public:
    MicrophoneHandler(int sdPin, int wsPin, int sckPin, int buttonPin);

    void begin();
    void update();
    bool isRecording();

private:
    int _sdPin, _wsPin, _sckPin, _buttonPin;
    bool _recording = false;

    void startRecording();
    void stopRecording();
    void readAudio();
};

#endif
