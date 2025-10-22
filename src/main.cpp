#include <Arduino.h>
#include <driver/i2s.h>
#include <FS.h>
#include <LittleFS.h>
#include <math.h>

#include "modules/MicrophoneHandler.h"
#include "modules/wififunc.h"       // уже существует
#include "modules/NetworkHandler.h" // пока не используется, но подключён для совместимости

// === Параметры ===
#define SAMPLE_RATE 16000
#define I2S_PORT I2S_NUM_0

// === Имена файлов ===
const char *WAV_FILENAME = "/record.wav";

// === Глобальные переменные ===
File wavFile;
uint32_t dataBytes = 0;
bool fileRecording = false;

// === WAV HEADER ===
struct WAVHeader
{
    char riff[4] = {'R', 'I', 'F', 'F'};
    uint32_t fileSize;
    char wave[4] = {'W', 'A', 'V', 'E'};
    char fmtChunk[4] = {'f', 'm', 't', ' '};
    uint32_t fmtChunkSize = 16;
    uint16_t audioFormat = 1;
    uint16_t numChannels = 1;
    uint32_t sampleRate = SAMPLE_RATE;
    uint32_t byteRate = SAMPLE_RATE * 2; // 16-bit mono → 2 bytes/sample
    uint16_t blockAlign = 2;
    uint16_t bitsPerSample = 16;
    char dataChunk[4] = {'d', 'a', 't', 'a'};
    uint32_t dataSize;
};

// === Функция записи заголовка WAV ===
void writeWavHeader(File &file, uint32_t dataSize)
{
    WAVHeader header;
    header.dataSize = dataSize;
    header.fileSize = dataSize + sizeof(WAVHeader) - 8;
    file.seek(0);
    file.write((const uint8_t *)&header, sizeof(WAVHeader));
}

// === Объекты проекта ===
MicrophoneHandler mic(GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_32, GPIO_NUM_34);
WiFiFunc wifi; // сеть уже инициализирована ранее, пока не используется

// === Инициализация ===
void setup()
{
    Serial.begin(115200);
    delay(1000);

    // --- ФС ---
    if (!LittleFS.begin(true))
    {
        Serial.println("[FS] LittleFS mount failed");
        return;
    }
    Serial.println("[FS] LittleFS mounted");

    // --- Микрофон ---
    mic.begin();
    Serial.println("[Mic] Ready. Hold button to record.");
}

// === Начать запись ===
void startRecording()
{
    fileRecording = true;
    Serial.println("[Rec] Recording started...");

    wavFile = LittleFS.open(WAV_FILENAME, FILE_WRITE);
    if (!wavFile)
    {
        Serial.println("[Rec] Failed to open file for writing");
        fileRecording = false;
        return;
    }

    // Резервируем 44 байта под заголовок WAV
    uint8_t emptyHeader[44] = {0};
    wavFile.write(emptyHeader, sizeof(emptyHeader));
    dataBytes = 0;
}

// === Остановить запись ===
void stopRecording()
{
    fileRecording = false;
    writeWavHeader(wavFile, dataBytes);
    wavFile.close();

    Serial.printf("[Rec] Saved %u bytes to %s\n", dataBytes, WAV_FILENAME);
}

// === Основной цикл ===
void loop()
{
    mic.update(); // отслеживает кнопку

    bool micState = mic.isRecording(); // кнопка зажата → true
    static bool prevState = false;

    // Старт / стоп
    if (micState && !prevState)
        startRecording();
    if (!micState && prevState)
        stopRecording();
    prevState = micState;

    // Если идёт запись — читаем I2S и сохраняем в файл
    if (fileRecording)
    {
        const int BUFFER_SAMPLES = 256;
        int32_t samples[BUFFER_SAMPLES];
        size_t bytesRead = 0;

        esp_err_t res = i2s_read(I2S_PORT, (void *)samples, sizeof(samples), &bytesRead, 1000);
        if (res == ESP_OK && bytesRead > 0)
        {
            int samplesRead = bytesRead / sizeof(int32_t);
            int16_t buffer16[BUFFER_SAMPLES];
            for (int i = 0; i < samplesRead; i++)
            {
                buffer16[i] = (int16_t)(samples[i] >> 11);
            }
            wavFile.write((uint8_t *)buffer16, samplesRead * sizeof(int16_t));
            dataBytes += samplesRead * 2;
        }
    }

    delay(10);
}
