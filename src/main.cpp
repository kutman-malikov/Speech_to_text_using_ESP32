#include <Arduino.h>
#include "modules/wififunc.h"

WiFiFunc wifi;

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Добавляем несколько Wi-Fi сетей
    wifi.addNetwork("Asronet", "20052007.");
    wifi.addNetwork("MobileHotspot", "myhotspot123");
    wifi.addNetwork("astronet", "20052007.");

    wifi.connect();             // пробуем подключиться
    wifi.startMonitorTask();    // запускаем фоновый поток
}

void loop() {
    // тут может выполняться логика распознавания речи, отправка данных и т.д.
    delay(5000);
}
