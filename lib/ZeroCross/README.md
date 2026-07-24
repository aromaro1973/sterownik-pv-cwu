# ZeroCross

Modul detekcji przejscia przez zero dla sieci AC.

## Funkcje

- obsluga przerwania `IRAM_ATTR` na pinie zero-cross,
- filtr szumow impulsowych (minimalny odstep czasowy),
- zliczanie impulsow i obliczanie czestotliwosci,
- wywolanie `PhaseController::trigger()` przy poprawnym przejsciu.

## Bezpieczenstwo danych

Licznik impulsow jest kopiowany w sekcji krytycznej (`noInterrupts()/interrupts()`), aby uniknac utraty danych przy jednoczesnym ISR.
