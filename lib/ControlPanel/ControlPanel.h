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

    // Gettery stanu (dla kompatybilności w trybie MAIN, jeśli potrzebne, 
    // ale teraz logikę przenosimy do main.cpp lub zarządzamy nią z zewnątrz)
    WorkMode getMode() const;
    void setMode(WorkMode mode);
    
    uint8_t getManualPower() const;
    void setManualPower(uint8_t power);

    // NOWOŚĆ: Funkcje sprawdzające, czy dany przycisk został właśnie KLIKNIĘTY
    // Odczytanie metody automatycznie zeruje flagę kliknięcia (czysty impuls)
    bool wasModePressed();
    bool wasPlusPressed();
    bool wasMinusPressed();

private:
    void readButtons();

    // Stan pracy przetrzymywany w klasie
    WorkMode m_mode;
    uint8_t  m_manualPower;

    // Flagi zdarzeń (impulsów) dla loop()
    bool m_modeClicked;
    bool m_plusClicked;
    bool m_minusClicked;

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