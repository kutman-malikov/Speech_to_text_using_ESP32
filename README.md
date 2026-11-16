# 🗣️ Speech-to-Text Translation System using ESP32

A standalone embedded system that records speech, sends it to the **ElevenLabs Speech-to-Text API**, translates the recognized text via **DeepL**, and displays the final translation on a **TFT display** — all powered by an **ESP32** microcontroller.

---

## 📘 Overview

This project shows how a low-cost microcontroller can perform multilingual speech recognition and translation using local audio recording and cloud-based AI services. The system captures audio, saves it as a WAV file, uploads it for recognition, obtains translated text, and presents the result to the user.

---

## ✨ Features

- 🎤 I²S audio recording via **INMP441 microphone**  
- 💾 Stores audio as **16-bit WAV** using **LittleFS**  
- 🌐 Cloud recognition through **ElevenLabs STT API**  
- 🌍 Multi-language translation through **DeepL**  
- 🖥️ Displays translation on **ILI9341 TFT (SPI)**  
- 📶 Continuous Wi-Fi monitoring using **FreeRTOS task**  
- 🔊 Built-in **local web server** to download/listen to recordings

---

## 🧩 Hardware Setup

| Component | Function | ESP32 Pins |
|----------|----------|------------|
| **INMP441 Microphone** | I²S audio capture | SCK=32, WS=25, SD=33, L/R=GND |
| **ILI9341 TFT Display** | Visual output | MOSI=23, MISO=19, SCK=18, CS=15, DC=2, RST=4 |
| **Push Button** | Start/stop recording | GPIO34 (active-low) |
| **Power** | USB 5 V | — |

> All modules share a common GND.

---

## 🧠 Software Architecture

Project uses a modular C++ structure under **PlatformIO**:

| Module | Description |
|--------|-------------|
| `MicrophoneHandler` | Configures I²S, reads 32-bit samples, converts to 16-bit PCM |
| `AudioRecorder` | Writes WAV to LittleFS and manages headers |
| `WiFiFunc` | Handles Wi-Fi connection + FreeRTOS background monitoring |
| `ElevenLabsSTT` | Uploads `/record.wav` for speech-to-text processing |
| `Translator` | Sends recognized text to DeepL API and returns translation |
| `DisplayHandler` | Handles all TFT output (states + final text) |
| `main.cpp` | Controls recording, processing, translations, and flow |

---

## 🔄 System Flow

```mermaid
flowchart LR

A[Start] --> B[Init Wi-Fi, Display, I2S]
B --> C{Button Pressed?}
C -->|Yes| D[Start Recording]
C -->|No| C

D --> E[Write 16-bit Samples to WAV]
E --> F[Button Released]
F --> G[Stop Recording]

G --> H[Upload to ElevenLabs]
H --> I[Receive Recognized Text]

I --> J[Send to DeepL]
J --> K[Receive Translation].

K --> L[Display Translation]
L --> M[Idle]
M --> C
```

---

## 🧪 Results

The system worked reliably during testing. Audio recording, cloud recognition, translation, and display output all functioned smoothly. Optimized I²S configuration ensured clean audio compatible with ElevenLabs. Wi-Fi remained stable thanks to FreeRTOS monitoring. The device consistently produced correct translations and responded quickly to user actions.

---

## 🧭 Future Improvements

- On-screen language selection  
- Basic history of past translations  
- Better error/status messages on the display

---

## 📂 Project Structure

```
Speech_to_text_ESP32/
 ├── src/
 │   ├── main.cpp
 │   └── modules/
 │       ├── AudioRecorder.*
 │       ├── MicrophoneHandler.*
 │       ├── WiFiFunc.*
 │       ├── ElevenLabsSTT.*
 │       ├── Translator.*
 │       └── DisplayHandler.*
 ├── include/
 ├── lib/
 ├── data/
 ├── platformio.ini
 └── README.md
```

---

## 🔧 How to Build

1. Install **PlatformIO** (VSCode extension recommended)  
2. Clone the repository  
3. Copy `include/secrets.example.h` to `include/secrets.h` and fill in your Wi-Fi credentials and API keys (`secrets.h` is git-ignored):
```cpp
#define WIFI_SSID "..."
#define WIFI_PASSWORD "..."
#define ELEVENLABS_API_KEY "..."
#define DEEPL_API_KEY "..."
```
4. Connect your ESP32 board  
5. Upload firmware  
6. Open Serial Monitor @ **115200 baud**

---

## 📎 Author

**Kutman Malikov**  
Electrical and Electronics Engineering  
Bahçeşehir Cyprus University