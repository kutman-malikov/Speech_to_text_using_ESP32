# ESP32 Speech-to-Text

A project for ESP32 to convert speech to text using a microphone and a cloud service (e.g., ElevenLabs).

## Description

This project allows ESP32 to capture audio from a microphone, process the signal, and send it to a server for speech recognition. Suitable for voice interfaces, smart devices, and IoT projects.

## Features

* Supports INMP441 microphone
* Real-time speech-to-text conversion
* Integration with ElevenLabs API
* Simple setup and modular architecture

## Requirements

* ESP32 (any compatible module)
* INMP441 or similar microphone
* Arduino IDE or PlatformIO
* Wi-Fi connection
* ElevenLabs API key

## Installation

1. Clone the repository:

```bash
git clone https://github.com/kutman-malikov/esp32-speech-to-text.git
```

2. Open the project in Arduino IDE or PlatformIO.

3. Install required libraries:

* WiFi
* HTTPClient
* I2S (for INMP441)

4. Configure Wi-Fi and API key in `config.h` (create from `config_template.h`).

## Usage

1. Connect ESP32 to your computer via USB.
2. Upload the sketch to the board.
3. Open Serial Monitor to see the recognized text.
4. Speak into the microphone and watch the results in the console.

## Project Structure

```
/src          # Main project code
/include      # Header files and configuration
/lib          # External libraries
README.md     # Project description
```

## License

MIT License — free to use, modify, and distribute.

## Contact

* Email: [kutman.engineer@gmail.com](mailto:kutman.engineer@gmail.com)
* GitHub: [kutman-malikov](https://github.com/kutman-malikov)
