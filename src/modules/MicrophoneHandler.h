#ifndef MICROPHONE_HANDLER_H
#define MICROPHONE_HANDLER_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <functional>

class MicrophoneHandler {
public:
    MicrophoneHandler(int sdPin, int wsPin, int sckPin, int buttonPin);

    void begin();
    void update();
    bool isRecording();

    void startRecording();
    void stopRecording();
    void readAudio();

    // callback для передачи сэмплов (используется рекордером)
    std::function<void(int16_t)> _onSample;

private:
    int _sdPin, _wsPin, _sckPin, _buttonPin;
    bool _recording = false;
};

#endif
