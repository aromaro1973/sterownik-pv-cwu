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

    // NOWOŚĆ: Zmiana ze String na const char* - gigantyczna oszczędność RAM i CPU
    static const char* boolToString(bool value);
    static const char* onOff(bool value);

    // Przeliczenie procentów na waty grzałki
    static float percentToPower(float percent, float maxPower);
};

#endif // UTILS_H