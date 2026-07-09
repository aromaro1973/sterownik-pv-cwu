#ifndef ESP_NOW_MANAGER_H
#define ESP_NOW_MANAGER_H

#include <Arduino.h>
#include "InverterPacket.h"

class ESPNowManager
{
public:
    ESPNowManager();

    void begin();
    void update();

    bool isConnected() const;
    uint32_t getPacketCounter() const;
    uint32_t getLastPacketTime() const;
    
    // Nowe gettery diagnostyczne:
    uint32_t getLostPackets() const;
    uint32_t getLastPeriodMs() const; // czas jaki upłynął między ostatnimi dwoma pakietami

    uint16_t getInverterPower() const;
    uint16_t getPVPower() const;
    int16_t  getBatteryPower() const; 
    uint16_t getHousePower() const;

    uint8_t  getSOC() const;
    float    getBatteryVoltage() const;
    float    getBatteryCurrent() const;

    void handleRx(const uint8_t* incomingData, int len);

private:
    bool     m_connected;
    uint32_t m_packetCounter;
    uint32_t m_lastPacketTime;
    
    // Nowe zmienne diagnostyczne:
    uint32_t m_lastPeriodMs;
    uint32_t m_lostPackets;
    uint32_t m_expectedPacketId;
    bool     m_firstPacketReceived;

    uint16_t m_inverterPower;
    uint16_t m_pvPower;
    int16_t  m_batteryPower;
    uint16_t m_housePower;

    uint8_t  m_soc;
    float    m_batteryVoltage;
    float    m_batteryCurrent;
};

#endif // ESP_NOW_MANAGER_H