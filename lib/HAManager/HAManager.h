#ifndef HA_MANAGER_H
#define HA_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "ESPNowManager.h"
#include "Guardian.h"
#include "ControlPanel.h"

class HAManager
{
public:
    HAManager(WiFiClient& wifiClient, ESPNowManager& espNow, Guardian& guardian, ControlPanel& cp);

    void begin(const char* mqttServer, uint16_t mqttPort, const char* mqttUser = nullptr, const char* mqttPass = nullptr);
    void update();

private:
    WiFiClient& m_wifiClient;
    PubSubClient m_mqttClient;
    ESPNowManager& m_espNow;
    Guardian& m_guardian;
    ControlPanel& m_controlPanel;

    const char* m_mqttServer;
    uint16_t m_mqttPort;
    const char* m_mqttUser;
    const char* m_mqttPass;

    uint32_t m_lastPublishTime;
    String m_deviceId;

    void reconnect();
    void sendDiscoveryConfig();
    void publishState();
    static void mqttCallback(char* topic, byte* payload, unsigned int length);
    void handleIncomingCommand(String topic, String message);
};

#endif // HA_MANAGER_H