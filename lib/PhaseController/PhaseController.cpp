#include "PhaseController.h"

uint8_t PhaseController::_triacPin = 0;
// Domyślnie 10000 us (10 ms) = triak całkowicie wyłączony
volatile uint32_t PhaseController::_delayMicros = 10000; 

void PhaseController::begin(uint8_t zeroCrossPin, uint8_t triacPin) {
    _triacPin = triacPin;
    pinMode(_triacPin, OUTPUT);
    digitalWrite(_triacPin, LOW);
    pinMode(zeroCrossPin, INPUT_PULLUP);
}

void PhaseController::setPower(int percent) {
    // Implementacja zabezpieczenia z dokumentacji (0-40% -> 100%)
    if (percent <= 0) {
        _delayMicros = 10000; // Wyłączony (czekaj cały półokres)
        return;
    }
    
    if (percent > 40 && percent < 95) {
        percent = 40; // Sztywne cięcie pasma zabronionego dla falownika
    }
    
    if (percent >= 95) {
        _delayMicros = 0; // Pełna moc, odpalaj od razu przy zerze
        return;
    }
    
    // Skalowanie bezpiecznego zakresu 1% - 40% 
    // Dla 1% opóźnienie ok. 9500 us, dla 40% ok. 6500 us (zbocze opadające)
    _delayMicros = map(percent, 0, 40, 10000, 6500);
}

uint32_t PhaseController::getDelayMicros() {
    return _delayMicros;
}