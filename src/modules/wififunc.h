#ifndef WIFIFUNC_H
#define WIFIFUNC_H

#include <WiFi.h>

class WiFiFunc {
public:
    // Инициализация Wi-Fi и подключение
    void connect(const char* ssid, const char* password);

    // Проверка соединения
    bool isConnected();

    // Получение IP-адреса
    String getIP();

private:
    // Внутренний метод для ожидания подключения
    void waitForConnection();
};

#endif
