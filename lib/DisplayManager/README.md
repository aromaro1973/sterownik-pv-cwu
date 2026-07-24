# DisplayManager

Modul renderuje LCD 16x2 po I2C i zarzadza struktur ekranow.

## Ekrany

- `SPLASH`
- `MAIN`
- `INFO` (1..7)
- `CONFIG` (1..4)

## Aktualna mapa

### INFO

1. ESP-NOW RADIO
2. ZEROCROSS
3. PHASE CTRL
4. AUTOCONTROL
5. MOC GRZANIA GRZAL
6. MOC FALOWNIKA
7. MOC BATERII

### CONFIG

1. MOC MAX FALOWNIK
2. MAX ROZLAD. BATER.
3. CZAS ZWLOKA PV
4. MOC GRZALKI

## Technika odswiezania

- odswiezanie warunkowe przez `m_refreshRequired`,
- ograniczenie odswiezania do 300 ms,
- wymuszone odswiezanie na ekranach INFO/CONFIG.
