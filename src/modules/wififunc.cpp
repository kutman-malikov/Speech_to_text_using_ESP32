#include "wififunc.h"
#include <Arduino.h>

void WiFiFunc::connect(const char* ssid, const char* password) {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    waitForConnection();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi connection failed.");
    }
}

bool WiFiFunc::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WiFiFunc::getIP() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not connected";
}

void WiFiFunc::waitForConnection() {
    int maxAttempts = 20;
    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < maxAttempts) {
        delay(500);
        Serial.print(".");
        attempt++;
    }
    Serial.println();
}
