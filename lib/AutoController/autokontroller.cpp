#include "AutoController.h"

AutoController::AutoController()
    : m_heaterPowerW(2000),
    m_requestedPowerW(0),
      m_currentPowerPercent(0),
    m_increaseHoldUntilMs(0)
{
}

void AutoController::begin(uint16_t nominalHeaterPower)
{
    m_heaterPowerW = nominalHeaterPower;
    reset();
}

void AutoController::setHeaterPower(uint16_t nominalHeaterPower)
{
    m_heaterPowerW = nominalHeaterPower;
}

uint16_t AutoController::getHeaterPower() const
{
    return m_heaterPowerW;
}

void AutoController::reset()
{
    m_requestedPowerW = 0;
    m_currentPowerPercent = 0;
    m_increaseHoldUntilMs = 0;
}

uint8_t AutoController::calculateOffGridPower(int32_t powerInv, int32_t powerBat,
                                              int32_t maxBatDischargeW, uint16_t inverterMaxPowerW, uint16_t stepPowerW,
                                              uint16_t pvHoldDelayMs)
{
    uint32_t now = millis();
    uint16_t controlStepW = stepPowerW == 0 ? 1 : stepPowerW;
    int32_t workingInverterLimitW = (int32_t)((uint32_t)inverterMaxPowerW * 90U / 100U);
    uint32_t holdDelay = pvHoldDelayMs == 0 ? 1 : pvHoldDelayMs;

    // Twarde zabezpieczenie: jeśli inwerter przekroczył limit użytkownika,
    // natychmiast wycofujemy obciążenie i zaczynamy analizę od zera.
    if (powerInv > inverterMaxPowerW)
    {
        reset();
        return 0;
    }

    int32_t batteryOverdrawW = powerBat - maxBatDischargeW;
    int32_t inverterOverdrawW = powerInv - workingInverterLimitW;

    // Zejście w dół jest natychmiastowe. Korekta wykorzystuje rzeczywisty nadmiar,
    // a nie stały procent, więc szybciej wraca do bezpiecznego punktu pracy.
    if (batteryOverdrawW > 0 || inverterOverdrawW > 0)
    {
        uint16_t correctionW = controlStepW;
        if (batteryOverdrawW > correctionW) {
            correctionW = batteryOverdrawW;
        }
        if (inverterOverdrawW > correctionW) {
            correctionW = inverterOverdrawW;
        }

        if (m_requestedPowerW > correctionW) {
            m_requestedPowerW -= correctionW;
        } else {
            m_requestedPowerW = 0;
        }
    }
    else if (now >= m_increaseHoldUntilMs && m_requestedPowerW < m_heaterPowerW)
    {
        // Wzrost mocy jest celowo powolny. Po każdym kroku dajemy PV czas
        // na dociągnięcie mocy, zanim ocenimy wpływ na baterię.
        uint32_t nextPowerW = (uint32_t)m_requestedPowerW + controlStepW;
        if (nextPowerW > m_heaterPowerW) {
            nextPowerW = m_heaterPowerW;
        }

        m_requestedPowerW = (uint16_t)nextPowerW;
        m_increaseHoldUntilMs = now + holdDelay;
    }

    m_currentPowerPercent = (uint8_t)(((uint32_t)m_requestedPowerW * 100U) / m_heaterPowerW);
    if (m_requestedPowerW > 0 && m_currentPowerPercent == 0) {
        m_currentPowerPercent = 1;
    }

    return m_currentPowerPercent;
}