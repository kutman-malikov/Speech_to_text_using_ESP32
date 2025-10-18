#include "Arduino.h"
#include "modules/wififunc.h"

WiFiFunc wifi;  // создаем объект Wi-Fi

// ⚠️ Замени на свои реальные данные
const char* SSID = "Astronet";
const char* PASSWORD = "20052007.";

void setup() {
    Serial.begin(115200);
    delay(1000);

    wifi.connect(SSID, PASSWORD);

    if (wifi.isConnected()) {
        Serial.println("Connected successfully!");
        Serial.print("Device IP: ");
        Serial.println(wifi.getIP());
    } else {
        Serial.println("Failed to connect to Wi-Fi.");
    }
}

void loop() {
    // Здесь пока ничего
}