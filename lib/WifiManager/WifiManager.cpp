#include "WiFiManager.h"

void WiFiManager::begin(const char *ssid, const char *password)
{
    wifiSSID = ssid;
    wifiPassword = password;

    WiFi.mode(WIFI_STA);
    
    // Włączamy automatyczne odnawianie połączenia na poziomie sprzętowym ESP32
    WiFi.setAutoReconnect(true); 
    WiFi.begin(wifiSSID, wifiPassword);

    connected = false;
    lastCheckTime = millis();
    disconnectTime = millis();
}

void WiFiManager::update()
{
    uint32_t now = millis();

    // Sprawdzamy stan sieci rzadko (np. co 1000ms), aby odciążyć pętlę loop()
    if (now - lastCheckTime >= 1000)
    {
        lastCheckTime = now;
        bool currentStatus = (WiFi.status() == WL_CONNECTED);

        if (currentStatus != connected)
        {
            connected = currentStatus;
            
            if (!connected)
            {
                // Zapamiętujemy moment, w którym faktycznie straciliśmy sieć
                disconnectTime = now; 
            }
        }

        // Twarda pętla ratunkowa (Watchdog Wi-Fi)
        // Jeśli ESP32 nie połączy się samo przez 30 sekund, pomagamy mu ręcznie
        if (!connected)
        {
            if (now - disconnectTime >= 30000)
            {
                disconnectTime = now; // Resetujemy timer awaryjny
                
                WiFi.disconnect();
                WiFi.begin(wifiSSID, wifiPassword);
            }
        }
    }
}

bool WiFiManager::isConnected() const
{
    return connected;
}

String WiFiManager::getIP() const
{
    if (!connected)
    {
        return "";
    }
    return WiFi.localIP().toString();
}

int32_t WiFiManager::getRSSI() const
{
    if (!connected)
    {
        return 0;
    }
    return WiFi.RSSI();
}