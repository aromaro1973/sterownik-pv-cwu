#ifndef ZERO_CROSS_H
#define ZERO_CROSS_H

#include <Arduino.h>

class ZeroCross {
public:
    void begin();
    void update();
    bool available();
    float getFrequency();

    // Metoda przerwania musi być statyczna i umieszczona w pamięci IRAM dla szybkości
    static void IRAM_ATTR isr();

private:
    static volatile bool _zeroCrossed;
    static volatile uint32_t _pulseCount;
    static volatile uint32_t _lastPulseMicros; // <-- DODANE DO FILTRACJI
    static uint32_t _lastPulseTime;
    static float _frequency;
};

#endif