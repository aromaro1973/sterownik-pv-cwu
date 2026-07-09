#ifndef AUTO_CONTROLLER_H
#define AUTO_CONTROLLER_H

#include <Arduino.h>

class AutoController
{
public:
    AutoController();

    // Inicjalizacja (moc grzałki w Watach, np. 2000W)
    void begin(uint16_t nominalHeaterPower);

    // Główny algorytm Off-Grid dla falownika Anenji
    // powerPV - moc paneli, powerInv - moc wyjściowa AC, powerBat - moc baterii (dodatnia = rozładowanie, ujemna = ładowanie)
    // maxBatDischargeW - bezpieczny próg rozładowania (np. 400W)
    uint8_t calculateOffGridPower(int32_t powerPV, int32_t powerInv, int32_t powerBat, 
                                  int32_t maxBatDischargeW, bool guardianBlocked);

    // Pełny reset algorytmu (powrót do procedury startowej)
    void reset();

private:
    uint16_t m_heaterPowerW;      // Moc znamionowa grzałki (W)
    uint8_t  m_currentPowerPercent; // Aktualny stopień wysterowania grzałki (0-100%)
    uint32_t m_lastRegTime;       // Czas ostatniego kroku regulacji
    bool     m_inSoftStart;       // Flaga procedury startowej (Pre-positioning)
};

#endif // AUTO_CONTROLLER_H