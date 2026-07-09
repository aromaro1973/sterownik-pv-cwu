#ifndef HEATER_OUTPUT_H
#define HEATER_OUTPUT_H

#include <Arduino.h>

class HeaterOutput
{
public:

    // Inicjalizacja wyjścia
    void begin();

    // Włączenie wyjścia
    void on();

    // Wyłączenie wyjścia
    void off();

    // Aktualny stan wyjścia
    bool getState() const;

private:

    bool state = false;
};

#endif