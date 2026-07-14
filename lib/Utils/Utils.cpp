#include <Utils.h>

volatile uint32_t Utils::zcCounter = 0;
volatile uint32_t Utils::triggerCounter = 0;

float Utils::clamp(float value, float minValue, float maxValue)
{
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

bool Utils::elapsed(uint32_t &previousMillis, uint32_t interval)
{
    uint32_t now = millis();

    if (now - previousMillis >= interval)
    {
        previousMillis = now;
        return true;
    }
    return false;
}

// Zwracamy wskaźnik do pamięci Flash - zero alokacji String!
const char* Utils::boolToString(bool value)
{
    return value ? "TRUE" : "FALSE";
}

const char* Utils::onOff(bool value)
{
    return value ? "ON" : "OFF";
}

float Utils::percentToPower(float percent, float maxPower)
{
    return (maxPower * percent) / 100.0f;
}