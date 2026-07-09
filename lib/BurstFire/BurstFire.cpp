#include "BurstFire.h"

BurstFire::BurstFire()
    : powerPercent(0),
      position(0)
{
}

void BurstFire::setPower(uint8_t percent)
{
    if (percent > 100)
    {
        percent = 100;
    }

    // JEŚLI MOC SIĘ ZMIENIŁA:
    // Resetujemy pozycję, aby zapobiec anomalion (glitchom) matematycznym w locie
    if (powerPercent != percent)
    {
        powerPercent = percent;
        position = 0; 
    }
}

bool BurstFire::next()
{
    // Algorytm dystrybucji impulsów Bresenhama
    bool state = ((position * powerPercent) / 100) !=
                 (((position + 1) * powerPercent) / 100);

    position++;

    if (position >= 100)
    {
        position = 0;
    }

    return state;
}