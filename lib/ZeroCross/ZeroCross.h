#ifndef ZERO_CROSS_H
#define ZERO_CROSS_H

#include <Arduino.h>

class ZeroCross
{
public:

    // Inicjalizacja modułu
    void begin();

    // Aktualizacja modułu (wywoływana w loop)
    void update();

    // Czy pojawił się nowy półokres?
    bool available();

    // Liczba półokresów z ostatniej sekundy
    uint32_t getHalfCycles() const;

    // Całkowity licznik półokresów
    uint32_t getTotalCounter() const;

    // Aktualna częstotliwość sieci
    float getFrequency() const;

    // Czy sygnał jest obecny
    bool isSignalPresent() const;

private:

    static void IRAM_ATTR isr();

    static volatile bool pulseDetected;
    static volatile uint32_t pulseCounter;
    static volatile uint32_t lastPulseMicros;

    uint32_t previousCounter = 0;
    uint32_t halfCycles = 0;

    uint32_t lastSecond = 0;

    float frequency = 0.0f;
    bool signalPresent = false;
};

#endif