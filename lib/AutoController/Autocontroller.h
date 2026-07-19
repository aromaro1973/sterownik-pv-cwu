#ifndef AUTO_CONTROLLER_H
#define AUTO_CONTROLLER_H

#include <Arduino.h>

/**
 * @class AutoController
 * @brief Moduł automatycznego śledzenia nadwyżek i zarządzania mocą grzałki.
 * 
 * Realizuje dwustopniową kontrolę:
 * 1. Wyznacza 15% strefy bezpieczeństwa od progu Guardiana i porównuje z mocą falownika.
 * 2. Weryfikuje bilans baterii w celu płynnego doregulowania lub zrzutu mocy przy chmurach.
 */
class AutoController
{
public:
    AutoController();
    void begin(uint16_t nominalHeaterPower);
    
    /**
     * @brief Główny algorytm EMS wyliczający optymalne wysterowanie grzałki
     * @param powerPV Aktualna generacja z paneli fotowoltaicznych [W]
     * @param powerInv Aktualna moc obciążenia falownika [W]
     * @param powerBat Bilans mocy akumulatora [W] (dodatnia = rozładowanie, ujemna = ładowanie)
     * @param maxBatDischargeW Maksymalne dopuszczalne rozładowanie baterii na cele grzałki [W]
     * @param guardianBlocked Flaga twardego odcięcia wygenerowana przez Guardian
     * @param guardianMaxPower Ustawiony w Guardianie limit mocy falownika [W]
     */
    uint8_t calculateOffGridPower(int32_t powerPV, int32_t powerInv, int32_t powerBat, 
                                  int32_t maxBatDischargeW, bool guardianBlocked, uint16_t guardianMaxPower);

    void reset();

private:
    uint16_t m_heaterPowerW; 
    uint8_t  m_currentPowerPercent; 
    uint32_t m_lastRegTime; 
    bool     m_inSoftStart; 
};

#endif // AUTO_CONTROLLER_H