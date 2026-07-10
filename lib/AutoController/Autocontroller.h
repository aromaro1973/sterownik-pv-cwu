#ifndef AUTO_CONTROLLER_H
#define AUTO_CONTROLLER_H

#include <Arduino.h>

/**
 * @class AutoController
 * @brief Moduł automatycznego śledzenia nadwyżek energii dla systemu Off-Grid.
 * * Implementuje hybrydowy algorytm regulacji krokowej (co 500ms):
 * - Płynna regulacja fazowa w bezpiecznym zakresie: 0% - 40%
 * - Strefa martwa (blokada generowania wyższych harmonicznych): 41% - 99%
 * - Przełączenie skokowe na pełną falę (czysta sinusoida): 100%
 */
class AutoController
{
public:
    AutoController();

    /**
     * @brief Inicjalizacja kontrolera automatycznego
     * @param nominalHeaterPower Moc znamionowa podłączonej grzałki w Watach (np. 2000W)
     */
    void begin(uint16_t nominalHeaterPower);

    /**
     * @brief Główny algorytm EMS wyliczający optymalne wysterowanie grzałki
     * @param powerPV Aktualna generacja z paneli fotowoltaicznych [W]
     * @param powerInv Sumaryczne obciążenie wyjściowe falownika AC [W]
     * @param powerBat Bilans mocy akumulatora [W] (dodatnia = rozładowanie, ujemna = ładowanie)
     * @param maxBatDischargeW Dopuszczalny próg rozładowania akumulatora na cele grzewcze (np. 400W)
     * @param guardianBlocked Flaga wymuszenia twardego odcięcia przez moduł Guardian
     * @return Rzeczywisty, bezpieczny procent wysterowania dla PhaseController (0-40 lub 100)
     */
    uint8_t calculateOffGridPower(int32_t powerPV, int32_t powerInv, int32_t powerBat, 
                                  int32_t maxBatDischargeW, bool guardianBlocked);

    /**
     * @brief Pełny reset maszyny stanów algorytmu (powrót do procedury szybkiego startu)
     */
    void reset();

private:
    uint16_t m_heaterPowerW;        ///< Zapamiętana moc znamionowa grzałki [W]
    uint8_t  m_currentPowerPercent; ///< Bieżący wyliczony stopień wysterowania (0-40% lub 100%)
    uint32_t m_lastRegTime;         ///< Znacznik czasu [ms] ostatniego wykonanego kroku regulacji
    bool     m_inSoftStart;         ///< Flaga aktywnej procedury szybkiego startu (Pre-positioning)
};

#endif // AUTO_CONTROLLER_H