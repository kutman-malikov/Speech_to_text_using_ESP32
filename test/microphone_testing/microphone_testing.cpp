#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

// === ПОДКЛЮЧЕНИЯ ===
// INMP441
#define I2S_SD  33   // SD (data)
#define I2S_WS  25   // WS (LRCL)
#define I2S_SCK 32   // SCK (BCLK)
// Кнопка записи
#define BUTTON_PIN 34

// === НАСТРОЙКИ ===
#define SAMPLE_RATE 16000
#define I2S_PORT I2S_NUM_0

// === ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ===
bool isRecording = false;

// === НАСТРОЙКА I2S ===
void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  // Устанавливаем драйвер
  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_set_sample_rates(I2S_PORT, SAMPLE_RATE);

  Serial.println("[Mic] I2S initialized.");
}

// === ЧТЕНИЕ И ОБРАБОТКА ===
void readAudio() {
  const int BUFFER_SAMPLES = 1024;
  int32_t samples[BUFFER_SAMPLES];
  size_t bytesRead = 0;

  esp_err_t res = i2s_read(I2S_PORT, (void*)samples, sizeof(samples), &bytesRead, 1000);

  if (res != ESP_OK || bytesRead == 0) {
    Serial.printf("[Mic] i2s_read failed (%d), bytes=%u\n", (int)res, (unsigned)bytesRead);
    return;
  }

  int samplesRead = bytesRead / sizeof(int32_t);

  int32_t peak = 0;
  uint64_t sumSquares = 0;

  for (int i = 0; i < samplesRead; i++) {
    int32_t s = samples[i] >> 8; // сдвигаем на 8 бит — теперь 24-бит реальный звук

    if (s < 0) s = -s;
    if (s > peak) peak = s;
    sumSquares += (uint64_t)(s * s);
  }

  float rms = sqrt((double)sumSquares / (double)samplesRead);
  int16_t rms16 = (int16_t)(rms / 256.0); // масштабируем к 16-бит
  int16_t peak16 = peak / 256;

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 300) {
    Serial.printf("[Mic] Peak16=%d | RMS16=%d\n", peak16, rms16);
    lastPrint = millis();
  }
}

// === НАСТРОЙКА И ОСНОВНОЙ ЦИКЛ ===
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT);
  setupI2S();
  Serial.println("[Mic] Ready. Hold button to record.");
}

void loop() {
  bool button = digitalRead(BUTTON_PIN);

  if (button && !isRecording) {
    isRecording = true;
    Serial.println("[Mic] Recording started...");
  }
  if (!button && isRecording) {
    isRecording = false;
    Serial.println("[Mic] Recording stopped.");
  }

  if (isRecording) {
    readAudio();
  }

  delay(10);
}
