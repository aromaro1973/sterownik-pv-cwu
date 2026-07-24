# Utils

Zestaw helperow wspolnych dla modulow.

## Zawartosc

- `clamp(...)`
- `elapsed(...)`
- konwersje tekstowe `boolToString/onOff`
- przeliczenie `percentToPower(...)`
- wspoldzielone liczniki diagnostyczne ISR:
  - `zcCounter`
  - `triggerCounter`

## Uwagi

Liczniki ISR sa oznaczone jako `volatile` i odczytywane atomowo po stronie petli glownej.
