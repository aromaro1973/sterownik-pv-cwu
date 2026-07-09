#include "ControlPanel.h"
#include "Config.h"

ControlPanel::ControlPanel()
    : m_mode(WorkMode::OFF),
      m_manualPower(0),
      m_modeClicked(false),
      m_plusClicked(false),
      m_minusClicked(false),
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

// Implementacja pobierania zdarzenia z auto-resetem (impuls jednorazowy)
bool ControlPanel::wasModePressed() { bool temp = m_modeClicked; m_modeClicked = false; return temp; }
bool ControlPanel::wasPlusPressed() { bool temp = m_plusClicked; m_plusClicked = false; return temp; }
bool ControlPanel::wasMinusPressed() { bool temp = m_minusClicked; m_minusClicked = false; return temp; }

void ControlPanel::readButtons()
{
    uint32_t now = millis();
    const uint32_t debounceDelay = 50; // 50 ms odfiltrowania drgań styków

    bool modeState  = digitalRead(PIN_BUTTON_MODE);
    bool plusState  = digitalRead(PIN_BUTTON_PLUS);
    bool minusState = digitalRead(PIN_BUTTON_MINUS);

    // MODE - Wykrywanie zbocza opadającego z debounce
    if (modeState != m_lastModeState)
    {
        if (m_lastModeState == HIGH && modeState == LOW)
        {
            if (now - m_lastModeTime > debounceDelay)
            {
                m_modeClicked = true;
                m_lastModeTime = now;
            }
        }
        m_lastModeState = modeState;
    }

    // PLUS
    if (plusState != m_lastPlusState)
    {
        if (m_lastPlusState == HIGH && plusState == LOW)
        {
            if (now - m_lastPlusTime > debounceDelay)
            {
                m_plusClicked = true;
                m_lastPlusTime = now;
            }
        }
        m_lastPlusState = plusState;
    }

    // MINUS
    if (minusState != m_lastMinusState)
    {
        if (m_lastMinusState == HIGH && minusState == LOW)
        {
            if (now - m_lastMinusTime > debounceDelay)
            {
                m_minusClicked = true;
                m_lastMinusTime = now;
            }
        }
        m_lastMinusState = minusState;
    }
}