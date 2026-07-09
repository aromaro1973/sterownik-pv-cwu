#ifndef INVERTER_PACKET_H
#define INVERTER_PACKET_H

#include <Arduino.h>

struct __attribute__((packed)) InverterPacket {
    uint32_t packetId;       // Linia 7: ID pakietu (zapobiega zgubionym ramkom)
    uint16_t pvPower;        // Linia 8: Moc z paneli PV
    uint16_t inverterPower;  // Linia 9: Moc oddawana przez falownik
    int16_t  batteryPower;   // Linia 10: Znak +/- wyliczony przez drugie ESP
    uint8_t  soc;            // Linia 11: Stan naładowania w %
    float    batteryVoltage; // Linia 12: Napięcie baterii
    float    batteryCurrent; // Linia 13: Prąd baterii
};

#endif // INVERTER_PACKET_H