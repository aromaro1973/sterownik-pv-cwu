#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager
{
public:
    void begin(const char *ssid, const char *password);
    void update();

    bool isConnected() const;
    String getIP() const;
    int32_t getRSSI() const;

private:
    const char *wifiSSID = nullptr;
    const char *wifiPassword = nullptr;
    bool connected = false;

    // NOWOŚĆ: Timery optymalizujące działanie sieci w tle
    uint32_t lastCheckTime = 0;
    uint32_t disconnectTime = 0;
};

#endif // WIFI_MANAGER_H