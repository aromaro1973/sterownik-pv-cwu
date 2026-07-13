#include "ControlPanel.h"
#include "Config.h"

ControlPanel::ControlPanel()
    : m_mode(WorkMode::OFF),
      m_manualPower(0),
      m_modeClicked(false),
      m_modeLongPressed(false),
      m_plusClicked(false),
      m_minusClicked(false),
      m_modeActive(false),
      m_modePressStartTime(0),
      m_lastModeState(HIGH),
      m_lastPlusState(HIGH),
      m_lastMinusState(HIGH),
      m_lastModeTime(0),
      m_lastPlusTime(0),
      m_lastMinusTime(0)
{
}

void ControlPanel::begin()
{
    pinMode(PIN_BUTTON_PLUS, INPUT_PULLUP);
    pinMode(PIN_BUTTON_MODE, INPUT_PULLUP);
    pinMode(PIN_BUTTON_MINUS, INPUT_PULLUP);
}

void ControlPanel::update()
{
    readButtons();
}

WorkMode ControlPanel::getMode() const { return m_mode; }
void ControlPanel::setMode(WorkMode mode) { m_mode = mode; }

uint8_t ControlPanel::getManualPower() const { return m_manualPower; }
void ControlPanel::setManualPower(uint8_t power) { m_manualPower = power; }

// Implementacja pobierania zdarzeń z auto-resetem
bool ControlPanel::wasModePressed() { bool temp = m_modeClicked; m_modeClicked = false; return temp; }
bool ControlPanel::wasModeLongPressed() { bool temp = m_modeLongPressed; m_modeLongPressed = false; return temp; }
bool ControlPanel::wasPlusPressed() { bool temp = m_plusClicked; m_plusClicked = false; return temp; }
bool ControlPanel::wasMinusPressed() { bool temp = m_minusClicked; m_minusClicked = false; return temp; }

void ControlPanel::readButtons()
{
    uint32_t now = millis();
    const uint32_t debounceDelay = 50; 
    const uint32_t longPressDuration = 2000; // Wymagany czas przytrzymania: 2 sekundy

    bool modeState  = digitalRead(PIN_BUTTON_MODE);
    bool plusState  = digitalRead(PIN_BUTTON_PLUS);
    bool minusState = digitalRead(PIN_BUTTON_MINUS);

    // =========================================================================
    // Zaawansowana obsługa przycisku MODE (Krótki klik vs Przytrzymanie 2s)
    // =========================================================================
    if (modeState != m_lastModeState)
    {
        if (now - m_lastModeTime > debounceDelay)
        {
            if (modeState == LOW) // Przycisk wciśnięty (zbocze opadające)
            {
                m_modeActive = true;
                m_modePressStartTime = now;
            }
            else // Przycisk puszczony (zbocze rosnące)
            {
                if (m_modeActive)
                {
                    // Jeśli przycisk został puszczony przed upływem 2 sekund -> traktujemy to jako krótki klik
                    if (now - m_modePressStartTime < longPressDuration)
                    {
                        m_modeClicked = true;
                    }
                    m_modeActive = false;
                }
            }
            m_lastModeTime = now;
        }
        m_lastModeState = modeState;
    }

    // Ciągły monitoring wciśnięcia przycisku MODE (reakcja bez konieczności puszczania)
    if (m_modeActive && !m_modeLongPressed)
    {
        if (now - m_modePressStartTime >= longPressDuration)
        {
            m_modeLongPressed = true;
            m_modeActive = false; // Zapobiega wielokrotnemu wyzwalaniu podczas jednego długiego trzymania
        }
    }

    // =========================================================================
    // Przycisk PLUS
    // =========================================================================
    if (plusState != m_lastPlusState)
    {
        if (now - m_lastPlusTime > debounceDelay)
        {
            if (plusState == LOW)
            {
                m_plusClicked = true;
            }
            m_lastPlusTime = now;
        }
        m_lastPlusState = plusState;
    }

    // =========================================================================
    // Przycisk MINUS
    // =========================================================================
    if (minusState != m_lastMinusState)
    {
        if (now - m_lastMinusTime > debounceDelay)
        {
            if (minusState == LOW)
            {
                m_minusClicked = true;
            }
            m_lastMinusTime = now;
        }
        m_lastMinusState = minusState;
    }
}