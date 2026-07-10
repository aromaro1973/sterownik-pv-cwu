#include "HAManager.h"
#include <Config.h> // Zakładam, że tu masz ew. definicje wersji lub interwałów

// Wskaźnik globalny do obsługi callbacku statycznego MQTT
static HAManager* haInstance = nullptr;

HAManager::HAManager(WiFiClient& wifiClient, ESPNowManager& espNow, Guardian& guardian, ControlPanel& cp)
    : m_wifiClient(wifiClient),
      m_mqttClient(wifiClient),
      m_espNow(espNow),
      m_guardian(guardian),
      m_controlPanel(cp),
      m_mqttServer(nullptr),
      m_mqttPort(1883),
      m_mqttUser(nullptr),
      m_mqttPass(nullptr),
      m_lastPublishTime(0)
{
    haInstance = this;
}

void HAManager::begin(const char* mqttServer, uint16_t mqttPort, const char* mqttUser, const char* mqttPass)
{
    m_mqttServer = mqttServer;
    m_mqttPort = mqttPort;
    m_mqttUser = mqttUser;
    m_mqttPass = mqttPass;

    // Generowanie unikalnego ID na podstawie adresu MAC
    m_deviceId = "sterownik_pv_" + WiFi.macAddress();
    m_deviceId.replace(":", "");
    m_deviceId.toLowerCase();

    m_mqttClient.setServer(m_mqttServer, m_mqttPort);
    m_mqttClient.setCallback(mqttCallback);
}

void HAManager::reconnect()
{
    if (!WiFi.isConnected()) return;

    if (!m_mqttClient.connected())
    {
        Serial.println("[MQTT] Próba połączenia z brokerem...");
        
        // Definicja tematu Last Will and Testament (Status dostępności urządzenia)
        String willTopic = "homeassistant/binary_sensor/" + m_deviceId + "/availability/state";
        
        if (m_mqttClient.connect(m_deviceId.c_str(), m_mqttUser, m_mqttPass, willTopic.c_str(), 1, true, "offline"))
        {
            Serial.println("[MQTT] Połączono!");
            m_mqttClient.publish(willTopic.c_str(), "online", true);
            
            // Rejestracja auto-discovery w Home Assistant
            sendDiscoveryConfig();

            // Subskrypcja tematów sterujących (komendy z HA)
            String cmdModeTopic = "sterownik_pv/" + m_deviceId + "/mode/set";
            String cmdPowerTopic = "sterownik_pv/" + m_deviceId + "/manual_power/set";
            m_mqttClient.subscribe(cmdModeTopic.c_str());
            m_mqttClient.subscribe(cmdPowerTopic.c_str());
        }
    }
}

void HAManager::sendDiscoveryConfig()
{
    String baseTopic = "homeassistant/";
    String deviceJson = ",\"dev\":{\"ids\":[\"" + m_deviceId + "\"],\"name\":\"Sterownik Nadwyżki PV\",\"mf\":\"AM\",\"md\":\"EMS Off-Grid\",\"sw\":\"0.3\"}";

    // 1. Konfiguracja Sensora: Tryb pracy (Select)
    String selectConfig = "{\"name\":\"Tryb Pracy\",\"cmd_t\":\"sterownik_pv/" + m_deviceId + "/mode/set\",\"stat_t\":\"sterownik_pv/" + m_deviceId + "/state\",\"val_tpl\":\"{{ value_json.mode }}\",\"options\":[\"OFF\",\"AUTO\",\"MANUAL\"]" + deviceJson + "}";
    m_mqttClient.publish((baseTopic + "select/" + m_deviceId + "/mode/config").c_str(), selectConfig.c_str(), true);

    // 2. Konfiguracja Sensora: Moc ręczna (Number)
    String numConfig = "{\"name\":\"Moc Ręczna\",\"cmd_t\":\"sterownik_pv/" + m_deviceId + "/manual_power/set\",\"stat_t\":\"sterownik_pv/" + m_deviceId + "/state\",\"val_tpl\":\"{{ value_json.manual_power }}\",\"min\":0,\"max\":100,\"step\":10,\"unit_of_meas\":\"%\"" + deviceJson + "}";
    m_mqttClient.publish((baseTopic + "number/" + m_deviceId + "/manual_power/config").c_str(), numConfig.c_str(), true);

    // 3. Konfiguracja Sensorów Telemetrycznych (Moc PV, Inv, Bat, Grzałka)
    auto publishSensorConfig = [this, &baseTopic, &deviceJson](String key, String name, String unit, String icon) {
        String config = "{\"name\":\"" + name + "\",\"stat_t\":\"sterownik_pv/" + m_deviceId + "/state\",\"val_tpl\":\"{{ value_json." + key + " }}\",\"unit_of_meas\":\"" + unit + "\",\"icon\":\"" + icon + "\"" + deviceJson + "}";
        m_mqttClient.publish((baseTopic + "sensor/" + m_deviceId + "_" + key + "/config").c_str(), config.c_str(), true);
    };

    publishSensorConfig("pv_power", "Moc PV", "W", "mdi:solar-power");
    publishSensorConfig("inv_power", "Obciążenie Falownika", "W", "mdi:home-lightning-bolt");
    publishSensorConfig("bat_power", "Bilans Baterii", "W", "mdi:battery-clock");
    publishSensorConfig("heater_percent", "Wysterowanie Grzałki", "%", "mdi:heat-wave");
    publishSensorConfig("lost_packets", "Zgubione Pakiety ESP-NOW", "pakiety", "mdi:wifi-alert");
    publishSensorConfig("wifi_rssi", "Sygnał WiFi RSSI", "dBm", "mdi:wifi");
}

void HAManager::publishState()
{
    if (!m_mqttClient.connected()) return;

    String modeStr = "OFF";
    if (m_controlPanel.getMode() == WorkMode::AUTO) modeStr = "AUTO";
    if (m_controlPanel.getMode() == WorkMode::MANUAL) modeStr = "MANUAL";

    // Wyliczenie szacowanej mocy grzałki w Watach (np. dla grzałki 2000W)
    uint16_t heaterWatts = 0;
    uint8_t currentPower = m_controlPanel.getManualPower();
    if (currentPower == 100) heaterWatts = 2000;
    else if (currentPower <= 40) heaterWatts = (2000 * currentPower) / 100;

    // Budowanie lekkiego JSON stanu bez użycia dynamicznych bibliotek (oszczędność RAM)
    String payload = "{";
    payload += "\"mode\":\"" + modeStr + "\",";
    payload += "\"manual_power\":" + String(currentPower) + ",";
    payload += "\"pv_power\":" + String(m_espNow.getPVPower()) + ",";
    payload += "\"inv_power\":" + String(m_espNow.getInverterPower()) + ",";
    payload += "\"bat_power\":" + String(m_espNow.getBatteryPower()) + ",";
    payload += "\"heater_percent\":" + String(currentPower) + ",";
    payload += "\"heater_watts\":" + String(heaterWatts) + ",";
    payload += "\"lost_packets\":" + String(m_espNow.getLostPackets()) + ",";
    payload += "\"wifi_rssi\":" + String(WiFi.RSSI());
    payload += "}";

    String stateTopic = "sterownik_pv/" + m_deviceId + "/state";
    m_mqttClient.publish(stateTopic.c_str(), payload.c_str());
}

void HAManager::update()
{
    if (!m_mqttClient.connected())
    {
        reconnect();
    }
    
    m_mqttClient.loop();

    // Publikacja danych raz na 1500ms, aby odciążyć sieć w tle
    if (millis() - m_lastPublishTime >= 1500)
    {
        m_lastPublishTime = millis();
        publishState();
    }
}

void HAManager::mqttCallback(char* topic, byte* payload, unsigned int length)
{
    if (haInstance != nullptr)
    {
        String message = "";
        for (unsigned int i = 0; i < length; i++) message += (char)payload[i];
        haInstance->handleIncomingCommand(String(topic), message);
    }
}

void HAManager::handleIncomingCommand(String topic, String message)
{
    if (topic.endsWith("/mode/set"))
    {
        WorkMode targetMode = WorkMode::OFF;
        if (message == "AUTO") targetMode = WorkMode::AUTO;
        if (message == "MANUAL") targetMode = WorkMode::MANUAL;

        m_controlPanel.setMode(targetMode);
        m_controlPanel.setManualPower(0);
        m_guardian.begin(2000); // Reset awaryjny guardiana przy zmianie trybu
        publishState(); // Natychmiastowe odświeżenie stanu w HA
    }
    else if (topic.endsWith("/manual_power/set"))
    {
        if (m_controlPanel.getMode() == WorkMode::MANUAL)
        {
            uint8_t targetPower = message.toInt();
            if (targetPower <= 100)
            {
                m_controlPanel.setManualPower(targetPower);
                publishState();
            }
        }
    }
}