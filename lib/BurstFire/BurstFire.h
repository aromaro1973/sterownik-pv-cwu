#ifndef BURST_FIRE_H
#define BURST_FIRE_H

#include <Arduino.h>

class BurstFire
{
public:
    BurstFire();

    // Ustawienie mocy 0...100%
    void setPower(uint8_t percent);

    // Wywoływane przy każdym półokresie (100 razy/s)
    // Zwraca:
    // true  -> załącz grzałkę
    // false -> wyłącz grzałkę
    bool next();

private:
    uint8_t powerPercent;
    uint8_t position;
};

#endif