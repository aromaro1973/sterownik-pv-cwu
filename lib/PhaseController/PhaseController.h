#ifndef PHASE_CONTROLLER_H
#define PHASE_CONTROLLER_H

#include <Arduino.h>

class PhaseController {
public:
    // Inicjalizacja pinu triaka i konfiguracja timera
    static void begin(uint8_t triacPin);
    
    // Ustawia moc od 0 do 100 (wywoływane w loop)
    static void setPower(int percent);

    // Zwraca rzeczywiście wykonany poziom mocy (0..47% fazowo lub 100% pełna fala z histerezą).
    static uint8_t getAppliedPower();
    
    // Zwraca aktualne opóźnienie w mikrosekundach
    static uint32_t getDelayMicros();
    
    // Ta funkcja będzie wywoływana bezpośrednio z przerwania w ZeroCross!
    static void IRAM_ATTR trigger(); 

private:
    static uint8_t _triacPin;
    static volatile uint32_t _delayMicros; 
    static volatile uint8_t _appliedPowerPercent;
    static bool _fullPowerLatched;
    static esp_timer_handle_t _timerHandle;

    // Metoda wywoływana przez timer po odliczeniu czasu opóźnienia
    static void IRAM_ATTR onTimerFire(void* arg);
};

#endif