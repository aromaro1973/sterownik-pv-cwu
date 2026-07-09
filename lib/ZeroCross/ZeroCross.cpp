#include <ZeroCross.h>
#include <Config.h>

volatile bool ZeroCross::pulseDetected = false;
volatile uint32_t ZeroCross::pulseCounter = 0;
volatile uint32_t ZeroCross::lastPulseMicros = 0;

void IRAM_ATTR ZeroCross::isr()
{
    uint32_t now = micros();

    // Filtr zakłóceń (5 ms)
    if (lastPulseMicros != 0)
    {
        if ((now - lastPulseMicros) < 5000)
            return;
    }

    lastPulseMicros = now;
    pulseCounter++;
    pulseDetected = true;
}

void ZeroCross::begin()
{
    pinMode(PIN_ZERO_CROSS, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(PIN_ZERO_CROSS),
        isr,
        FALLING);

    lastSecond = millis();
}

void ZeroCross::update()
{
    uint32_t now = millis();
    uint32_t localLastPulseMicros;

    // Bezpieczny odczyt volatile z przerwania
    noInterrupts();
    localLastPulseMicros = lastPulseMicros;
    interrupts();

    // Czy sygnał nadal istnieje?
    if ((micros() - localLastPulseMicros) > 50000)
    {
        signalPresent = false;
    }
    else
    {
        signalPresent = true;
    }

    // Aktualizacja raz na sekundę
    if (now - lastSecond >= 1000)
    {
        lastSecond = now;

        noInterrupts();
        uint32_t total = pulseCounter;
        interrupts();

        halfCycles = total - previousCounter;
        previousCounter = total;

        frequency = halfCycles / 2.0f;
    }
}

bool ZeroCross::available()
{
    // Sekcja krytyczna dla odczytu i resetu flagi przerwania
    if (pulseDetected)
    {
        noInterrupts();
        pulseDetected = false;
        interrupts();
        return true;
    }

    return false;
}

uint32_t ZeroCross::getHalfCycles() const
{
    return halfCycles;
}

uint32_t ZeroCross::getTotalCounter() const
{
    uint32_t total;
    
    noInterrupts();          // Wywołujemy bezpośrednio - makro zadziała globalnie
    total = pulseCounter;
    interrupts();           // Włączamy z powrotem przerwania
    
    return total;
}

float ZeroCross::getFrequency() const
{
    return frequency;
}

bool ZeroCross::isSignalPresent() const
{
    return signalPresent;
}