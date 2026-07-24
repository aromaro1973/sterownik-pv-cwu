# Config

Plik `Config.h` zawiera stale kompilacyjne projektu.

## Zakres

- wersje firmware/hardware,
- mapowanie pinow ESP32,
- stale czasowe i sieciowe,
- tryby pracy (`WorkMode`),
- ustawienia logowania.

## Uwaga

Wartosci runtime (limity i parametry menu) sa utrzymywane przez `Guardian` w NVS i moga nadpisywac stale domyslne po starcie.
