#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <Arduino.h>
#include <Config.h>

class ControlPanel
{
public:
    ControlPanel();

    void begin();
    void update();

    WorkMode getMode() const;
    void setMode(WorkMode mode);
    
    uint8_t getManualPower() const;
    void setManualPower(uint8_t power);

    // Funkcje sprawdzające zdarzenia (odczyt automatycznie zeruje flagę)
    bool wasModePressed();
    bool wasModeLongPressed(); // <-- NOWOŚĆ: Detekcja przytrzymania 2s
    bool wasPlusPressed();
    bool wasMinusPressed();

private:
    void readButtons();

    WorkMode m_mode;
    uint8_t  m_manualPower;

    // Flagi zdarzeń (impulsów) dla loop()
    bool m_modeClicked;
    bool m_modeLongPressed;    // <-- NOWOŚĆ
    bool m_plusClicked;
    bool m_minusClicked;

    // Wsparcie dla długiego kliknięcia MODE
    bool m_modeActive;             // Flaga czy przycisk MODE jest aktualnie trzymany
    uint32_t m_modePressStartTime; // Czas wciśnięcia przycisku MODE

    // Aktualny stan fizyczny pinów do wykrywania zboczy
    bool m_lastModeState;
    bool m_lastPlusState;
    bool m_lastMinusState;

    // Czas ostatniej zmiany (debounce)
    uint32_t m_lastModeTime;
    uint32_t m_lastPlusTime;
    uint32_t m_lastMinusTime;
};

#endif // CONTROL_PANEL_H