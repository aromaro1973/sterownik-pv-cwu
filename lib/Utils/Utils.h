#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

class Utils
{
public:
    // Ograniczenie wartości do zadanego przedziału
    static float clamp(float value, float minValue, float maxValue);

    // Nieblokujący timer (bezpieczny przy przepełnieniu millis)
    static bool elapsed(uint32_t &previousMillis, uint32_t interval);

    // Zmiana ze String na const char* - gigantyczna oszczędność RAM i CPU
    static const char* boolToString(bool value);
    static const char* onOff(bool value);

    // Przeliczenie procentów na waty grzałki
    static float percentToPower(float percent, float maxPower);

    // =========================================================================
    // NOWOŚĆ: Liczniki diagnostyczne (współdzielone między Core 0 a Core 1)
    // =========================================================================
    static volatile uint32_t zcCounter;       // Licznik impulsów przejścia przez zero
    static volatile uint32_t triggerCounter;  // Licznik fizycznych wyzwoleń triaka
};

#endif // UTILS_H