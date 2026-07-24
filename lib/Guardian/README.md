# Guardian

Modul odpowiada glownie za utrzymanie i persystencje parametrow konfiguracji w NVS.

## Parametry utrzymywane w NVS

- `maxPower`
- `powerStep`
- `maxBatteryDraw`
- `heaterPower`
- `pvHoldDelay`

## Rola runtime

- udostepnia wartosci dla `main.cpp` i `AutoController`,
- zawiera metody `update/isBlocked`, ale w aktualnej architekturze glowne zabezpieczenie komunikacyjne realizuje `main.cpp`.
