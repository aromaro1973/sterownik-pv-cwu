#include "AutoController.h"

AutoController::AutoController()
    : m_heaterPowerW(2000),
      m_currentPowerPercent(0),
      m_lastRegTime(0),
      m_inSoftStart(true)
{
}

void AutoController::begin(uint16_t nominalHeaterPower)
{
    m_heaterPowerW = nominalHeaterPower;
    reset();
}

void AutoController::reset()
{
    m_currentPowerPercent = 0;
    m_inSoftStart = true;
    m_lastRegTime = millis();
}

uint8_t AutoController::calculateOffGridPower(int32_t powerPV, int32_t powerInv, int32_t powerBat, 
                                              int32_t maxBatDischargeW, bool guardianBlocked, uint16_t guardianMaxPower)
{
    // Bezwzględne odcięcie awaryjne z Guardiana
    if (guardianBlocked)
    {
        if (m_currentPowerPercent > 0 || !m_inSoftStart)
        {
            reset();
        }
        return 0;
    }

    // -------------------------------------------------------------------------
    // KROK 1: Obliczenie strefy bezpieczeństwa (15% marginesu od limitu Guardiana)
    // -------------------------------------------------------------------------
    int32_t invWarningThresholdW = (int32_t)(guardianMaxPower * 0.85f);

    // Inteligenta pozycja startowa (Pre-positioning) przy włączeniu
    if (m_inSoftStart && m_currentPowerPercent == 0)
    {
        if (powerInv < invWarningThresholdW && powerBat < 0) 
        {
            int32_t initialSurplusW = abs(powerBat);
            uint32_t calculatedPercent = (initialSurplusW * 100) / m_heaterPowerW;
            
            if (calculatedPercent > 47 && calculatedPercent < 90)
            {
                m_currentPowerPercent = 47;
            }
            else if (calculatedPercent >= 90)
            {
                if ((powerInv + m_heaterPowerW) < invWarningThresholdW) {
                    m_currentPowerPercent = 100;
                } else {
                    m_currentPowerPercent = 47;
                }
            }
            else
            {
                m_currentPowerPercent = (uint8_t)calculatedPercent;
            }

            m_lastRegTime = millis();
            return m_currentPowerPercent;
        }
    }

    uint32_t now = millis();
    
    // Wykonanie właściwego kroku regulacji co 500ms
    if (now - m_lastRegTime >= 500)
    {
        m_lastRegTime = now;

        // -------------------------------------------------------------------------
        // KROK 2: Porównanie aktualnej mocy falownika z wyliczoną strefą bezpieczeństwa
        // -------------------------------------------------------------------------
        if (powerInv >= invWarningThresholdW)
        {
            // Przekroczyliśmy bezpieczną strefę falownika -> Natychmiastowa reakcja ochronna
            if (m_currentPowerPercent == 100)
            {
                m_currentPowerPercent = 47; // Natychmiastowy spadek do strefy fazowej
            }
            else
            {
                // Szybkie liniowe ściąganie mocy w dół w zakresie fazowym (krok ~200W)
                uint8_t stepDownPercent = (200 * 100) / m_heaterPowerW; 
                if (stepDownPercent < 8) stepDownPercent = 8;

                if (m_currentPowerPercent >= stepDownPercent) m_currentPowerPercent -= stepDownPercent;
                else                                         m_currentPowerPercent = 0;
            }
            m_inSoftStart = false;
            return m_currentPowerPercent; // Przerywamy, ochrona falownika ma priorytet
        }

        // -------------------------------------------------------------------------
        // KROK 3: Sprawdzenie baterii (Regulacja na podstawie warunków pogodowych / PV)
        // -------------------------------------------------------------------------
        if (powerBat > maxBatDischargeW)
        {
            // Bateria rozładowuje się zbyt mocno (np. chmura zasłoniła panele)
            if (m_currentPowerPercent == 100)
            {
                m_currentPowerPercent = 47; // Nagły spadek wydajności PV -> zrzut do 47%
            }
            else
            {
                // Płynniejsza redukcja krokowa w strefie fazowej (krok ~150W)
                uint8_t stepDownPercent = (150 * 100) / m_heaterPowerW; 
                if (stepDownPercent < 5) stepDownPercent = 5;

                if (m_currentPowerPercent >= stepDownPercent) m_currentPowerPercent -= stepDownPercent;
                else                                         m_currentPowerPercent = 0;
            }
            m_inSoftStart = false;
        }
        else
        {
            // Falownik ma luz i akumulator się ładuje -> Zwiększamy obciążenie grzałki
            if (m_currentPowerPercent < 47)
            {
                uint8_t stepUpPercent = (80 * 100) / m_heaterPowerW; // Płynny krok w górę ~80W
                if (stepUpPercent < 4) stepUpPercent = 4;

                m_currentPowerPercent += stepUpPercent;
                if (m_currentPowerPercent > 47) m_currentPowerPercent = 47;
            }
            else if (m_currentPowerPercent == 47)
            {
                // Warunek skoku na 100%: 
                // Zapas falownika musi pomieścić resztę mocy grzałki, a bateria musi mieć duży prąd ładowania
                int32_t addedPowerW = (m_heaterPowerW * 53) / 100; 
                
                bool invHasRoom = (powerInv + addedPowerW) < invWarningThresholdW;
                bool batHasSurplus = powerBat < -(addedPowerW * 0.7f);

                if (invHasRoom && batHasSurplus)
                {
                    m_currentPowerPercent = 100; // Skok na czystą sinusoidę
                }
            }
        }
    }

    return m_currentPowerPercent;
}