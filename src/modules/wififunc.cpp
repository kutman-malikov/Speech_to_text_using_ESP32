#include "wififunc.h"
#include <Arduino.h>

void WiFiFunc::addNetwork(const char* ssid, const char* password) {
    networks.push_back({ssid, password});
}

void WiFiFunc::connect() {
    if (networks.empty()) {
        Serial.println("[WiFi] No saved networks!");
        return;
    }

    for (auto& net : networks) {
        Serial.printf("[WiFi] Trying to connect to %s...\n", net.ssid.c_str());
        WiFi.begin(net.ssid.c_str(), net.password.c_str());

        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 8000) {
            delay(500);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("[WiFi] Connected to %s\n", net.ssid.c_str());
            Serial.print("[WiFi] IP: ");
            Serial.println(WiFi.localIP());
            return;
        } else {
            Serial.printf("[WiFi] Failed to connect to %s\n", net.ssid.c_str());
        }
    }

    Serial.println("[WiFi] Could not connect to any network.");
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

// === Фоновый мониторинг ===
void WiFiFunc::startMonitorTask() {
    if (monitorTaskHandle == NULL) {
        xTaskCreatePinnedToCore(
            monitorTask,
            "WiFiMonitorTask",
            4096,
            this,
            1,
            &monitorTaskHandle,
            1
        );
        Serial.println("[WiFi] Monitor task started");
    }
}

void WiFiFunc::monitorTask(void* parameter) {
    WiFiFunc* wifi = static_cast<WiFiFunc*>(parameter);
    wifi->monitorConnection();
}

void WiFiFunc::monitorConnection() {
    while (true) {
        if (!isConnected()) {
            Serial.println("[WiFi] Lost connection! Reconnecting...");
            connect();
        }
        vTaskDelay(10000 / portTICK_PERIOD_MS); // проверка каждые 10 секунд
    }
}
