#ifndef WIFIFUNC_H
#define WIFIFUNC_H

#include <WiFi.h>
#include <vector>
#include <string>

class WiFiFunc {
public:
    // Добавление сетей
    void addNetwork(const char* ssid, const char* password);

    // Подключение к WiFi
    void connect();

    // Проверка подключения
    bool isConnected();

    // Получить IP
    String getIP();

    // Запуск фонового потока мониторинга
    void startMonitorTask();

private:
    struct Network {
        String ssid;
        String password;
    };

    std::vector<Network> networks;
    TaskHandle_t monitorTaskHandle = NULL;

    void monitorConnection();   // функция потока
    static void monitorTask(void* parameter);  // статическая оболочка
};

#endif
