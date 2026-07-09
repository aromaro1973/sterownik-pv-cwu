#ifndef GUARDIAN_H
#define GUARDIAN_H

#include <Arduino.h>

enum class GuardianBlockReason
{
    NONE,
    MAX_POWER,
    POWER_STEP
};

class Guardian
{
public:
    // Inicjalizacja modułu - warto podać fizyczną moc grzałki z Config.h
    void begin(uint16_t nominalHeaterPower);

    // Aktualizacja modułu w loop()
    void update();

    // Parametry użytkownika (w Watach)
    void setMaxPower(uint16_t power);
    uint16_t getMaxPower() const;

    void setPowerStep(uint16_t step);
    uint16_t getPowerStep() const;

    // Stan modułu
    bool isBlocked() const;
    GuardianBlockReason getBlockReason() const;
    
    // NOWOŚĆ: Szybka konwersja powodu blokady na tekst dla Loggera/LCD
    const char* blockReasonToString() const;

private:
    uint16_t heaterMaxPower = 2000; // Fizyczna moc grzałki, np. 2000W
    uint16_t maxPower = 3500;       // Maksymalna dozwolona moc w instalacji
    uint16_t powerStep = 1000;      // Maksymalny skok mocy (ochrona przed udarami)

    bool blocked = false;
    GuardianBlockReason blockReason = GuardianBlockReason::NONE;
};

#endif // GUARDIAN_H