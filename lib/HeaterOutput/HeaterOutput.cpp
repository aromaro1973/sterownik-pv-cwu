#include <HeaterOutput.h>
#include <Config.h>

//==================================================
// Inicjalizacja
//==================================================

void HeaterOutput::begin()
{
    pinMode(PIN_TRIAC, OUTPUT);

    off();
}

//==================================================
// Włączenie wyjścia
//==================================================

void HeaterOutput::on()
{
    digitalWrite(PIN_TRIAC, HIGH);
    state = true;
}

//==================================================
// Wyłączenie wyjścia
//==================================================

void HeaterOutput::off()
{
    digitalWrite(PIN_TRIAC, LOW);
    state = false;
}

//==================================================
// Odczyt stanu
//==================================================

bool HeaterOutput::getState() const
{
    return state;
}