#include "Guardian.h"

void Guardian::begin(uint16_t nominalHeaterPower)
{
    heaterMaxPower = nominalHeaterPower;
    blocked = false;
    blockReason = GuardianBlockReason::NONE;
}

void Guardian::update()
{
    // Tutaj w przyszłości zaimplementujesz np.:
    // float currentPowerW = (actualPowerPercent * heaterMaxPower) / 100.0f;
    // if (currentPowerW > maxPower) { blocked = true; blockReason = GuardianBlockReason::MAX_POWER; }
}

void Guardian::setMaxPower(uint16_t power)
{
    maxPower = power;
}

uint16_t Guardian::getMaxPower() const
{
    return maxPower;
}

void Guardian::setPowerStep(uint16_t step)
{
    powerStep = step;
}

uint16_t Guardian::getPowerStep() const
{
    return powerStep;
}

bool Guardian::isBlocked() const
{
    return blocked;
}

GuardianBlockReason Guardian::getBlockReason() const
{
    return blockReason;
}

const char* Guardian::blockReasonToString() const
{
    switch (blockReason)
    {
        case GuardianBlockReason::NONE:       return "NONE";
        case GuardianBlockReason::MAX_POWER:  return "LIMIT MAX POWER";
        case GuardianBlockReason::POWER_STEP: return "LIMIT POWER STEP";
    }
    return "UNKNOWN";
}