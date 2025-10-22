#include <Arduino.h>
#include "modules/wififunc.h"
#include "modules/MicrophoneHandler.h"

MicrophoneHandler mic(GPIO_NUM_33, GPIO_NUM_25, GPIO_NUM_32, GPIO_NUM_34);
WiFiFunc wifi;

void setup() {
    Serial.begin(115200);
    delay(200); // Ждем стабилизации Serial
    mic.begin();
    // Добавляем несколько Wi-Fi сетей
    // wifi.addNetwork("astronet", "20052007.");
    // wifi.connect();             // пробуем подключиться
    // wifi.startMonitorTask();    // запускаем фоновый поток

}

void loop() {
    // Основной цикл может быть пустым, всё управление в задачах
    mic.update();
    delay(10);
}
