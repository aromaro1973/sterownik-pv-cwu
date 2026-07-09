#ifndef PHASE_CONTROLLER_H
#define PHASE_CONTROLLER_H

#include <Arduino.h>

class PhaseController {
public:
    // Inicjalizacja pinów dla detektora zera i triaka
    static void begin(uint8_t zeroCrossPin, uint8_t triacPin);
    
    // Ustawia moc od 0 do 100 (wywoływane w pętli loop na Core 1)
    static void setPower(int percent);
    
    // Zwraca aktualne opóźnienie w mikrosekundach
    static uint32_t getDelayMicros();

private:
    static uint8_t _triacPin;
    // Zmienna współdzielona między rdzeniami musi być volatile!
    static volatile uint32_t _delayMicros; 
};

#endif