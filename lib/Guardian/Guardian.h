#ifndef GUARDIAN_H
#define GUARDIAN_H

#include <Arduino.h>
#include <Preferences.h>

enum class BlockReason {
    NONE = 0,
    OVERLOAD = 1,
    NO_ZERO_CROSS = 2,
    HARDWARE_FAULT = 3
};

class Guardian
{
public:
    Guardian();

    void begin(uint16_t nominalHeaterPower);
    
    // Szybka aktualizacja wywoływana w krytycznych punktach programu
    void update(int32_t currentInverterPowerW, uint8_t currentTriacPercent);

    bool isBlocked() const;
    BlockReason getBlockReason() const;
    void resetBlock();

    void setMaxPower(uint16_t maxPower);
    uint16_t getMaxPower() const;

    void setPowerStep(uint16_t step);
    uint16_t getPowerStep() const;

    void setMaxBatteryDraw(uint16_t maxBatteryDrawW);
    uint16_t getMaxBatteryDraw() const;

    void setNominalHeaterPower(uint16_t nominalHeaterPower);
    uint16_t getNominalHeaterPower() const;

    void setPvHoldDelay(uint16_t holdDelayMs);
    uint16_t getPvHoldDelay() const;

    // Funkcje obsługi pamięci nieulotnej flash (NVS)
    void saveSettings();
    void loadSettings();

private:
    bool m_isBlocked;
    BlockReason m_blockReason;
    uint16_t m_maxInverterPowerW;
    uint16_t m_powerStep;
    uint16_t m_maxBatteryDrawW;
    uint16_t m_nominalHeaterPowerW;
    uint16_t m_pvHoldDelayMs;
};

#endif // GUARDIAN_H