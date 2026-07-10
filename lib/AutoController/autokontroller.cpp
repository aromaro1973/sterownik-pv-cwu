#include "AutoController.h"
#include "Utils.h"

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
                                              int32_t maxBatDischargeW, bool guardianBlocked)
{
    // 1. Bezwzględna blokada bezpieczeństwa (Guardian)
    if (guardianBlocked)
    {
        if (m_currentPowerPercent > 0 || !m_inSoftStart)
        {
            reset();
        }
        return 0;
    }

    // =====================================================================
    // PUNKT STARTOWY: Inteligentny start z nadwyżki (Pre-positioning)
    // =====================================================================
    if (m_inSoftStart && m_currentPowerPercent == 0)
    {
        if (powerBat < 0) // Bateria jest ładowana = mamy czystą nadwyżkę
        {
            int32_t initialSurplusW = abs(powerBat);
            uint32_t calculatedPercent = (initialSurplusW * 100) / m_heaterPowerW;
            
            // Logika hybrydowa na starcie: jeśli nadwyżka od razu przekracza limit 40%, 
            // ale nie pozwala na stabilne 100%, pozycjonujemy bezpiecznie na max limit fazowy (40%)
            if (calculatedPercent > 40 && calculatedPercent < 90)
            {
                m_currentPowerPercent = 40;
            }
            else if (calculatedPercent >= 90)
            {
                m_currentPowerPercent = 100;
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
    
    // Pętla regulacji krokowej (co 500ms) dopasowana do czasu reakcji falownika
    if (now - m_lastRegTime >= 500)
    {
        m_lastRegTime = now;

        // 2. REAKCJA ANTY-CZAJNIK (Nagłe obciążenie sieci przez inny odbiornik)
        if (powerBat > (maxBatDischargeW + 500)) 
        {
            // Jeśli byliśmy na 100%, natychmiast uciekamy do bezpiecznej strefy fazowej
            if (m_currentPowerPercent == 100)
            {
                m_currentPowerPercent = 40;
            }
            else if (m_currentPowerPercent >= 15) 
            {
                m_currentPowerPercent -= 15; // Szybki zrzut w dół w strefie fazowej
            }
            else 
            {
                m_currentPowerPercent = 0;
            }
            
            m_inSoftStart = false; 
            return m_currentPowerPercent;
        }

        // 3. REGULACJA STANDARDOWYCH WAHAŃ
        if (powerBat > maxBatDischargeW)
        {
            // Magazyn rozładowuje się zbyt mocno -> Zmniejszamy moc grzałki
            if (m_currentPowerPercent == 100)
            {
                // Zrzut ze 100% do progu granicznego sterowania fazowego
                m_currentPowerPercent = 40;
            }
            else
            {
                uint8_t stepDownPercent = (150 * 100) / m_heaterPowerW; // krok o ok. 150W
                if (stepDownPercent < 5) stepDownPercent = 5;

                if (m_currentPowerPercent >= stepDownPercent) m_currentPowerPercent -= stepDownPercent;
                else                                          m_currentPowerPercent = 0;
            }
            m_inSoftStart = false;
        }
        else
        {
            // Magazyn bezpieczny (ładowanie) -> Podnosimy moc grzałki
            if (m_currentPowerPercent < 40)
            {
                uint8_t stepUpPercent = (80 * 100) / m_heaterPowerW; // płynny krok o ok. 80W
                if (stepUpPercent < 4) stepUpPercent = 4;

                m_currentPowerPercent += stepUpPercent;
                if (m_currentPowerPercent > 40) m_currentPowerPercent = 40;
            }
            else if (m_currentPowerPercent == 40)
            {
                // Jesteśmy na ścianie limitu fazowego (40%).
                // Przełączenie na 100% nastąpi TYLKO wtedy, gdy prąd ładowania akumulatora
                // nadal wykazuje duży zapas mocy (np. akumulator jest intensywnie ładowany mocą > 60% brakującej mocy grzałki)
                int32_t remainingPowerW = (m_heaterPowerW * 60) / 100;
                if (powerBat < -(remainingPowerW * 0.7f)) // 70% marginesu histerezy
                {
                    m_currentPowerPercent = 100; // Skok na pełną falę
                }
            }
        }
    }

    return m_currentPowerPercent;
}