#include "PhaseController.h"
#include <Utils.h> // Wymagane do zliczania Utils::triggerCounter dla diagnostyki

uint8_t PhaseController::_triacPin = 0;
volatile uint32_t PhaseController::_delayMicros = 10000; 
esp_timer_handle_t PhaseController::_timerHandle;

void PhaseController::begin(uint8_t triacPin) {
    _triacPin = triacPin;
    pinMode(_triacPin, OUTPUT);
    digitalWrite(_triacPin, LOW);

    // Konfiguracja sprzętowego timera wysokiej rozdzielczości na ESP32 (esp_timer)
    esp_timer_create_args_t timerArgs = {};
    timerArgs.callback = &PhaseController::onTimerFire;
    timerArgs.name = "triac_timer";
    
    esp_timer_create(&timerArgs, &_timerHandle);
}

// Wywoływane natychmiast przy wykryciu zera (w przerwaniu ZeroCross::isr)
void IRAM_ATTR PhaseController::trigger() {
    // Zabezpieczenie: na początku cyklu upewniamy się, że pin jest wyłączony
    digitalWrite(_triacPin, LOW);

    uint32_t delayVal = _delayMicros;

    if (delayVal >= 9800) {
        // Moc 0% - triak zostaje wyłączony, nie odpalamy stopera
        return;
    }

    if (delayVal <= 50) {
        // Moc 100% - odpalamy triak od razu i trzymamy stan wysoki
        digitalWrite(_triacPin, HIGH);
        Utils::triggerCounter++;
        return;
    }

    // Odliczanie czasu opóźnienia do zapłonu triaka
    esp_timer_stop(_timerHandle);
    esp_timer_start_once(_timerHandle, delayVal);
}

// Wywoływane automatycznie przez sprzęt po odliczeniu delayVal mikrosekund
void IRAM_ATTR PhaseController::onTimerFire(void* arg) {
    // Generujemy precyzyjny impuls szpilkowy o długości 200 mikrosekund
    digitalWrite(_triacPin, HIGH);
    delayMicroseconds(200); 
    digitalWrite(_triacPin, LOW);
    
    Utils::triggerCounter++;
}

void PhaseController::setPower(int percent) {
    if (percent <= 0) {
        _delayMicros = 10000;
        return;
    }
    
    // Nowe sztywne cięcie – blokujemy moc powyżej 47%, dopóki nie osiągnie 95%
    if (percent > 47 && percent < 95) {
        percent = 47; 
    }
    
    if (percent >= 95) {
        _delayMicros = 0;
        return;
    }
    
    // Mapowanie zakresu 0-47% na mikrosekundy (9100 us to minimum, 5500 us to maksimum mocy na zboczu opadającym)
    _delayMicros = map(percent, 0, 47, 9100, 5500); 
}

uint32_t PhaseController::getDelayMicros() {
    return _delayMicros;
}