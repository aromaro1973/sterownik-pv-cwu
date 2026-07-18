#include <Config.h>
#include "ZeroCross.h"
#include "PhaseController.h" 
#include <Utils.h>           

// Definicje zmiennych statycznych
volatile bool ZeroCross::_zeroCrossed = false;
volatile uint32_t ZeroCross::_pulseCount = 0;
volatile uint32_t ZeroCross::_lastPulseMicros = 0; // <-- Inicjalizacja nowej zmiennej
uint32_t ZeroCross::_lastPulseTime = 0;
float ZeroCross::_frequency = 0.0f;

void ZeroCross::begin() {
    pinMode(PIN_ZERO_CROSS, INPUT_PULLUP); 
    attachInterrupt(digitalPinToInterrupt(PIN_ZERO_CROSS), ZeroCross::isr, FALLING);
    _lastPulseTime = millis();
}

// BŁYSKAWICZNA FUNKCJA PRZERWANIA Z FILTREM ZAKŁÓCEŃ
void IRAM_ATTR ZeroCross::isr() {
    uint32_t now = micros();

    // Filtr zakłóceń (4 ms) - zapobiega szumom szpilkowym na sieci
    if (_lastPulseMicros != 0) {
        if ((now - _lastPulseMicros) < 4000) { 
            return; // Ignoruj zbyt szybkie, fałszywe impulsy
        }
    }
    _lastPulseMicros = now;

    _zeroCrossed = true;
    _pulseCount++;
    Utils::zcCounter++; 

    // Natychmiast informujemy PhaseController o realnym przejściu przez zero
    PhaseController::trigger(); 
}

void ZeroCross::update() {
    // Obliczanie częstotliwości sieci raz na sekundę
    uint32_t now = millis();
    if (now - _lastPulseTime >= 1000) {
        // Obliczamy częstotliwość na podstawie odfiltrowanych impulsów
        _frequency = (_pulseCount / 2.0f) * (1000.0f / (now - _lastPulseTime));
        _pulseCount = 0;
        _lastPulseTime = now;
    }
}

bool ZeroCross::available() {
    if (_zeroCrossed) {
        _zeroCrossed = false;
        return true;
    }
    return false;
}

float ZeroCross::getFrequency() {
    if (millis() - _lastPulseTime > 2000) {
        return 0.0f; // Brak prądu / błąd detektora
    }
    return _frequency;
}