#include "ESPNowManager.h"
#include <esp_now.h>
#include <WiFi.h>

//=============================================================================
// Mechanizm mostka (Wrapper) dla obsługi callbacku ESP-NOW w C++
//=============================================================================
static ESPNowManager* instance = nullptr;

// ZMIANA: Klasyczna sygnatura (mac zamiast recvInfo)
void onDataRecv(const uint8_t* mac, const uint8_t* data, int len) 
{
    if (instance != nullptr) 
    {
        // Przekierowanie surowych bajtów do wnętrza obiektu klasy
        instance->handleRx(data, len);
    }
}

//=============================================================================
// Implementacja metod klasy ESPNowManager
//=============================================================================
// ... [cała reszta pliku, czyli konstruktor, begin(), handleRx() itd. zostaje dokładnie tak jak była] ...

//=============================================================================
// Implementacja metod klasy ESPNowManager
//=============================================================================

ESPNowManager::ESPNowManager()
    : m_connected(false),
      m_packetCounter(0),
      m_lastPacketTime(0),
      m_lastPeriodMs(0),
      m_lostPackets(0),
      m_expectedPacketId(0),
      m_firstPacketReceived(false),
      m_inverterPower(0),
      m_pvPower(0),
      m_batteryPower(0),
      m_housePower(0),
      m_soc(0),
      m_batteryVoltage(0.0f),
      m_batteryCurrent(0.0f)
{
}

void ESPNowManager::begin()
{
    // Przypisanie bieżącej instancji do wskaźnika globalnego
    instance = this;

    m_connected = false;
    m_packetCounter = 0;
    m_lostPackets = 0;
    m_firstPacketReceived = false;
    m_lastPacketTime = millis();

    if (esp_now_init() != ESP_OK) return;
    
    // Rejestracja naszego bezpiecznego wrappera
    esp_now_register_recv_cb(onDataRecv);
}

void ESPNowManager::handleRx(const uint8_t* incomingData, int len)
{
    if (len == sizeof(InverterPacket)) 
    {
        InverterPacket packet;
        memcpy(&packet, incomingData, sizeof(InverterPacket));

        uint32_t now = millis();
        
        // 1. Obliczanie interwału między pakietami
        if (m_packetCounter > 0) {
            m_lastPeriodMs = now - m_lastPacketTime;
        }

        // 2. Wyliczanie zgubionych pakietów (QoS)
        if (!m_firstPacketReceived) {
            m_firstPacketReceived = true;
            m_expectedPacketId = packet.packetId + 1;
        } else {
            if (packet.packetId > m_expectedPacketId) {
                m_lostPackets += (packet.packetId - m_expectedPacketId);
            }
            m_expectedPacketId = packet.packetId + 1;
        }

        // Przypisanie odchudzonych danych z falownika
        m_pvPower       = packet.pvPower;
        m_inverterPower = packet.inverterPower;
        m_batteryPower  = packet.batteryPower; 
        m_housePower    = packet.inverterPower; 

        m_packetCounter++;
        m_lastPacketTime = now;
        m_connected = true;
    }
}

void ESPNowManager::update()
{
    if (m_connected && (millis() - m_lastPacketTime > 7000)) 
    {
        m_connected = false;
        m_batteryPower = 0; 
        // Resetujemy flagę synchronizacji ID po utracie połączenia
        m_firstPacketReceived = false; 
    }
}

// Implementacja nowych getterów diagnostycznych
bool ESPNowManager::isConnected() const { return m_connected; }
uint32_t ESPNowManager::getPacketCounter() const { return m_packetCounter; }
uint32_t ESPNowManager::getLastPacketTime() const { return m_lastPacketTime; }
uint32_t ESPNowManager::getLostPackets() const { return m_lostPackets; }
uint32_t ESPNowManager::getLastPeriodMs() const { return m_lastPeriodMs; }

// Starsze gettery (zwracające sparsowane dane)
uint16_t ESPNowManager::getInverterPower() const { return m_inverterPower; }
uint16_t ESPNowManager::getPVPower() const { return m_pvPower; }
int16_t  ESPNowManager::getBatteryPower() const { return m_batteryPower; }
uint16_t ESPNowManager::getHousePower() const { return m_housePower; }
uint8_t  ESPNowManager::getSOC() const { return m_soc; }
float    ESPNowManager::getBatteryVoltage() const { return m_batteryVoltage; }
float    ESPNowManager::getBatteryCurrent() const { return m_batteryCurrent; }