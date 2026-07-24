# PhaseController

Modul wykonawczy triaka, zsynchronizowany z przejsciem przez zero.

## Zasada pracy

- przyjmuje komende mocy `0..100`,
- mapuje zakres `0..47%` na opoznienie zaplonu,
- zakres `48..99%` ogranicza do `47%`,
- `100%` uruchamia pelna fale,
- utrzymuje histereze `fullPowerLatched` i schodzi z 100% dopiero po spadku komendy ponizej 80%.

## Diagnostyka

- udostepnia `getAppliedPower()` oraz `getDelayMicros()`.
