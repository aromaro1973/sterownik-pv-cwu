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
    // 1. Jeśli Guardian zablokował system, bezwzględnie resetujemy automatykę i gasimy grzałkę
    if (guardianBlocked)
    {
        if (m_currentPowerPercent > 0 || !m_inSoftStart)
        {
            reset();
        }
        return 0;
    }

    // =====================================================================
    // PUNKT STARTOWY: Inteligentny start z sumy nadwyżki (Pre-positioning)
    // =====================================================================
    if (m_inSoftStart && m_currentPowerPercent == 0)
    {
        // Jeśli bateria jest ładowana (powerBat < 0), oznacza to czystą nadwyżkę PV.
        // Od razu wskakujemy na moc odpowiadającą temu ładowaniu, zamiast tykać od zera.
        if (powerBat < 0)
        {
            int32_t initialSurplusW = abs(powerBat); // Pobieramy wartość dodatnią nadwyżki
            
            // Przeliczamy Waty nadwyżki na procenty naszej grzałki
            m_currentPowerPercent = (initialSurplusW * 100) / m_heaterPowerW;
            m_currentPowerPercent = (uint8_t)Utils::clamp((float)m_currentPowerPercent, 0.0f, 100.0f);
            
            m_lastRegTime = millis(); // Dajemy falownikowi czas na ustabilizowanie po skoku
            return m_currentPowerPercent;
        }
    }

    uint32_t now = millis();
    
    // Pętla regulacji krokowej (co 500ms) - dopasowanie do czasu reakcji falownika
    if (now - m_lastRegTime >= 500)
    {
        m_lastRegTime = now;

        // 2. KROK REAKCJI (ANTY-CZAJNIK): Nagłe, potężne rozładowanie baterii
        // Jeśli akumulator oddaje znacznie więcej niż założony bezpieczny próg (np. próg + 500W)
        if (powerBat > (maxBatDischargeW + 500)) 
        {
            // Ostry zrzut mocy grzałki w dół o 30%, aby natychmiast odciążyć układ
            if (m_currentPowerPercent >= 30) m_currentPowerPercent -= 30;
            else                              m_currentPowerPercent = 0;
            
            m_inSoftStart = false; // Przerywamy soft-start, przechodzimy w tryb twardej stabilizacji
            return m_currentPowerPercent;
        }

        // 3. REGULACJA STANDARDOWYCH WAHAŃ
        if (powerBat > maxBatDischargeW)
        {
            // Magazyn rozładowuje się mocniej niż dopuszczasz (np. > 400W) -> Płynnie zmniejszamy moc grzałki o ~200W
            uint8_t stepDownPercent = (200 * 100) / m_heaterPowerW;
            if (stepDownPercent < 10) stepDownPercent = 10; 

            if (m_currentPowerPercent >= stepDownPercent) m_currentPowerPercent -= stepDownPercent;
            else                                          m_currentPowerPercent = 0;

            m_inSoftStart = false; // Kończymy soft-start, skoro musieliśmy cofać moc
        }
        else
        {
            // Magazyn bezpieczny (ładowanie lub rozładowanie < 400W) -> Płynnie podnosimy moc grzałki o ~100W
            if (m_currentPowerPercent < 100)
            {
                uint8_t stepUpPercent = (100 * 100) / m_heaterPowerW; 
                if (stepUpPercent < 5) stepUpPercent = 5;

                m_currentPowerPercent += stepUpPercent;
                if (m_currentPowerPercent > 100) m_currentPowerPercent = 100;
            }
        }
    }

    return m_currentPowerPercent;
}