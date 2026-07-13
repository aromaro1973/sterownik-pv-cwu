#ifndef INVERTER_PACKET_H
#define INVERTER_PACKET_H

#include <Arduino.h>

/**
 * @struct InverterPacket
 * @brief Odchudzona struktura ramki danych telemerycznych przesyłana przez ESP-NOW.
 * * Rozmiar struktury po optymalizacji to dokładnie 10 bajtów.
 * Przeznaczona do bezpośredniego mapowania surowego bufora pamięci (Zero-Copy).
 */
struct __attribute__((packed)) InverterPacket {
    uint32_t packetId;       // ID pakietu (Inkrementacja monotoniczna do kontroli QoS)
    uint16_t pvPower;        // Bieżąca moc generowana przez panele PV [W]
    uint16_t inverterPower;  // Obciążenie wyjściowe AC falownika (pobór domu) [W]
    int16_t  batteryPower;   // Bilans mocy akumulatora [W] (ujemna = ładowanie, dodatnia = rozładowanie)
};

#endif // INVERTER_PACKET_H