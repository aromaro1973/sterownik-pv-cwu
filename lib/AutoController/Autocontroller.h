#ifndef AUTO_CONTROLLER_H
#define AUTO_CONTROLLER_H

#include <Arduino.h>

/**
 * @class AutoController
 * @brief Regulator mocy AUTO pracujący wewnętrznie na watach.
 *
 * Zasada:
 * - szybkie zejście mocy przy przekroczeniu progów,
 * - krokowe podnoszenie mocy po czasie zwłoki PV,
 * - zwrot komendy w procentach 0..100 dla PhaseController.
 */
class AutoController
{
public:
    AutoController();
    void begin(uint16_t nominalHeaterPower);
    void setHeaterPower(uint16_t nominalHeaterPower);
    uint16_t getHeaterPower() const;
    
    /**
     * @brief Główny algorytm EMS wyliczający optymalne wysterowanie grzałki
     * @param powerInv Aktualna moc obciążenia falownika [W]
     * @param powerBat Bilans mocy akumulatora [W] (dodatnia = rozładowanie, ujemna = ładowanie)
     * @param maxBatDischargeW Maksymalne dopuszczalne rozładowanie baterii na cele grzałki [W]
     * @param inverterMaxPowerW Ustawiony limit mocy falownika [W]
     * @param stepPowerW Krok regulacji mocy [W]
    * @param pvHoldDelayMs Zwłoka przed kolejnym krokiem podbicia mocy [ms]
     */
    uint8_t calculateOffGridPower(int32_t powerInv, int32_t powerBat,
                                  int32_t maxBatDischargeW, uint16_t inverterMaxPowerW, uint16_t stepPowerW,
                                  uint16_t pvHoldDelayMs);

    void reset();

private:
    uint16_t m_heaterPowerW; 
    uint16_t m_requestedPowerW;
    uint8_t  m_currentPowerPercent; 
    uint32_t m_increaseHoldUntilMs;
};

#endif // AUTO_CONTROLLER_H