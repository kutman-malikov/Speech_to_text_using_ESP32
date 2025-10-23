#include "modules/AudioFilter.h"
#include <math.h>

// === DSP параметры ===
static float hp_alpha = 0.995f;   // High-pass
static float lp_alpha = 0.25f;    // Low-pass
static float prevIn = 0, prevOut = 0, lpPrev = 0;
static float gain = 0.7f;         // начальное усиление (авто регулируется)

// === Вспомогательные функции ===
static inline float softClip(float x) {
    float n = x / 28000.0f;
    float y = tanh(n) * 28000.0f;
    return y;
}

// === Главная функция ===
int16_t AudioFilter::process(int32_t raw) {
    // ↓ преобразуем 24-бит → 16-бит и немного уменьшаем усиление
    float s = (float)(raw >> 8) * gain;

    // Dither
    s += ((float)random(-64, 64) / 256.0f);

    // High-pass (удаляет гул)
    float hp = s - prevIn + hp_alpha * prevOut;
    prevIn = s;
    prevOut = hp;

    // Low-pass (убирает шипение)
    float lp = lp_alpha * hp + (1.0f - lp_alpha) * lpPrev;
    lpPrev = lp;

    // --- Автоматическая регулировка усиления (AGC-lite) ---
    float absVal = fabs(lp);
    float target = 12000.0f; // желаемый средний уровень
    float adjust = 0.0002f * (target - absVal);
    gain += adjust;
    gain = constrain(gain, 0.3f, 1.0f);

    // --- Soft clip ---
    float clipped = softClip(lp);

    return (int16_t)constrain(clipped, -32768.0f, 32767.0f);
}
