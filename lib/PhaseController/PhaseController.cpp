#include "PhaseController.h"
#include <Utils.h> // Wymagane do zliczania Utils::triggerCounter dla diagnostyki

// Inicjalizacja składowych statycznych klasy
uint8_t PhaseController::_triacPin = 0;
volatile uint32_t PhaseController::_delayMicros = 10000; 
volatile uint8_t PhaseController::_appliedPowerPercent = 0;
bool PhaseController::_fullPowerLatched = false;
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

// Wywoływane bezpośrednio z przerwania w ZeroCross (public)
void IRAM_ATTR PhaseController::trigger() {
    // 1. Bezwzględne zatrzymanie stopera z poprzedniej półfali.
    // Zapobiega to nakładaniu się przerwaniami przy szumach na linii zasilania.
    esp_timer_stop(_timerHandle);
    
    // Upewniamy się, że zdejmujemy stan wysoki z bramki triaka
    digitalWrite(_triacPin, LOW);

    uint32_t delayVal = _delayMicros;

    if (delayVal >= 9800) {
        // Moc 0% - triak zostaje wyłączony, nie odpalamy stopera
        return;
    }

    if (delayVal == 0) {
        // Moc 100% - odpalamy triak od razu i trzymamy stan wysoki przez całą półfalę
        digitalWrite(_triacPin, HIGH);
        Utils::triggerCounter++;
        return;
    }

    // 2. Odliczanie czasu opóźnienia do zapłonu triaka (sterowanie fazowe)
    esp_timer_start_once(_timerHandle, delayVal);
}

// Metoda prywatna (private), wywoływana automatycznie przez sprzęt po odliczeniu delayVal
void IRAM_ATTR PhaseController::onTimerFire(void* arg) {
    // 3. Włączamy bramkę triaka
    digitalWrite(_triacPin, HIGH);
    
    // 4. Generujemy stabilny impuls szpilkowy o długości 50 mikrosekund.
    // Pozwala to na pewne otworzenie się struktury triaka.
    ets_delay_us(50); 
    
    // 5. Wyłączamy prąd bramki. Zgodnie z fizyką półprzewodników, 
    // triak będzie przewodził sam, dopóki prąd nie spadnie do zera (koniec półfali).
    digitalWrite(_triacPin, LOW);
    
    Utils::triggerCounter++;
}

void PhaseController::setPower(int percent) {
    if (percent <= 0) {
        _fullPowerLatched = false;
        _appliedPowerPercent = 0;
        _delayMicros = 10000;
        return;
    }

    if (percent >= 100) {
        _fullPowerLatched = true;
        _appliedPowerPercent = 100;
        _delayMicros = 0;
        return;
    }

    if (_fullPowerLatched) {
        if (percent >= 80) {
            _appliedPowerPercent = 100;
            _delayMicros = 0;
            return;
        }

        _fullPowerLatched = false;
    }

    if (percent > 47) {
        percent = 47;
    }

    _appliedPowerPercent = (uint8_t)percent;

    // Mapowanie zakresu 0-47% na mikrosekundy 
    // (9100 us to minimum, 5500 us to maksimum mocy na zboczu opadającym)
    _delayMicros = map(percent, 0, 47, 9100, 5500); 
}

uint32_t PhaseController::getDelayMicros() {
    return _delayMicros;
}

uint8_t PhaseController::getAppliedPower() {
    return _appliedPowerPercent;
}