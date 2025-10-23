#ifndef AUDIO_FILTER_H
#define AUDIO_FILTER_H

#include <Arduino.h>

class AudioFilter {
public:
    static int16_t process(int32_t rawSample);
};

#endif
