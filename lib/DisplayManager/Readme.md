# DisplayManager

## Zadanie modułu

Moduł `DisplayManager` odpowiada za obsługę wyświetlacza LCD 2×16 I²C.

Wyświetla aktualny stan pracy sterownika oraz podstawowe parametry pracy grzałki i sieci energetycznej.

---

## Funkcje

- inicjalizacja wyświetlacza LCD 2×16
- wyświetlanie trybu pracy:
  - OFF
  - AUTO
  - MANUAL
- wyświetlanie aktualnej mocy grzałki
- wyświetlanie stanu grzałki (ON/OFF)
- wyświetlanie liczby impulsów BurstFire
- wyświetlanie częstotliwości sieci
- odświeżanie wyświetlacza tylko po zmianie danych

---

## Wejścia

Moduł odbiera informacje z pozostałych modułów projektu:

```cpp
setMode()
setPower()
setHeaterState()
setBurst()
setFrequency()
```

---

## Wyświetlane informacje

### Linia 1

```
MANUAL 100% OFF
```

- tryb pracy
- moc grzałki
- stan grzałki

### Linia 2

```
BUR:100 FR:50.0
```

- liczba impulsów BurstFire
- częstotliwość sieci

---

## Układ pól LCD

### Linia 1

| Pozycje | Zawartość |
|---------:|-----------|
| 1–7 | Tryb pracy |
| 8 | Spacja |
| 9–12 | Moc (%) |
| 13 | Spacja |
| 14–16 | ON / OFF |

### Linia 2

| Pozycje | Zawartość |
|---------:|-----------|
| 1–7 | BUR:xxx |
| 8 | Spacja |
| 9–15 | FR:xx.x |
| 16 | Rezerwa |

---

## Współpraca z modułami

```
ControlPanel
       │
AutoController
       │
Guardian
       │
       ▼
DisplayManager
       │
       ▼
LCD 2×16
```

---

## Moduł nie odpowiada za

- sterowanie grzałką,
- algorytm BurstFire,
- pomiar częstotliwości,
- obsługę przycisków,
- komunikację WiFi,
- Home Assistant.

---

## Testy

Moduł został sprawdzony podczas testów:

- poprawne uruchomienie LCD 2×16,
- poprawne wyświetlanie:
  - trybu pracy,
  - mocy,
  - stanu ON/OFF,
  - BurstFire,
  - częstotliwości,
- poprawna współpraca z modułem `ControlPanel`,
- poprawna współpraca z modułem `BurstFire`,
- test z żarówką 100 W,
- test z grzałką 1800 W.

---

## Status

✅ Ukończony

---

## Autor

Arkadiusz Marek

Projekt rozwijany przy współpracy z ChatGPT.

---

## Wersja modułu

**1.0**