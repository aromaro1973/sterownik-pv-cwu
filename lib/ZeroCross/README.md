# ZeroCross

## Zadanie modułu

Moduł `ZeroCross` odpowiada za wykrywanie przejścia napięcia sieciowego przez zero.

Na podstawie sygnału z optoizolatora H11AA1 moduł wykrywa każdy półokres sieci, mierzy częstotliwość oraz informuje pozostałe moduły o pojawieniu się nowego półokresu.

---

## Funkcje

- detekcja przejścia przez zero
- obsługa przerwania ESP32
- filtracja zakłóceń
- zliczanie półokresów
- pomiar częstotliwości sieci
- wykrywanie zaniku sygnału sieci
- informowanie o nowym półokresie

---

## Wejścia

Sygnał z układu H11AA1 podłączonego do wejścia ESP32.

---

## Wyjścia

Moduł udostępnia:

```cpp
available()

getHalfCycles()

getTotalCounter()

getFrequency()

isSignalPresent()
```

---

## Współpraca z modułami

```
H11AA1
   │
   ▼
ZeroCross
   │
   ├────────► BurstFire
   │
   ├────────► DisplayManager
   │
   ├────────► Logger
   │
   └────────► Guardian
```

---

## Aktualna logika pracy

1. Wykrycie przejścia przez zero.
2. Wywołanie przerwania ESP32.
3. Odfiltrowanie zakłóceń (5 ms).
4. Zwiększenie licznika półokresów.
5. Ustawienie informacji o nowym półokresie.
6. Aktualizacja częstotliwości raz na sekundę.
7. Wykrywanie zaniku sygnału sieci.

---

## Moduł nie odpowiada za

- sterowanie triakiem,
- regulację mocy,
- algorytm BurstFire,
- wyświetlanie danych,
- komunikację WiFi,
- automatykę sterownika.

---

## Parametry

- częstotliwość sieci: 50 Hz
- liczba półokresów: 100/s
- filtr zakłóceń: 5 ms
- wykrywanie zaniku sygnału: 50 ms

---

## Testy

Moduł został sprawdzony podczas testów:

- poprawna detekcja przejścia przez zero,
- poprawna współpraca z H11AA1,
- poprawna filtracja zakłóceń,
- stabilny pomiar:
  - HalfCycles = 100,
  - Frequency = 50.0 Hz,
- poprawna współpraca z BurstFire,
- poprawna współpraca z HeaterOutput,
- poprawna współpraca z DisplayManager,
- poprawna współpraca z Logger,
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

**2.0**