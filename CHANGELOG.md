## v0.2.0

### Dodano

- Zakończono moduł `ZeroCross` (V2).
- Dodano programową filtrację zakłóceń (5 ms) w obsłudze przerwania.
- Dodano automatyczne wykrywanie obecności sygnału sieci.
- Dodano pomiar liczby półokresów w ostatniej sekundzie.
- Dodano stabilny pomiar częstotliwości sieci.
- Moduł `ZeroCross` sam oblicza częstotliwość i liczbę półokresów.
- Uproszczono `main.cpp` poprzez przeniesienie logiki do modułu `ZeroCross`.
- Zakończono moduł `BurstFire`.
- Dodano moduł `DisplayManager`.
- Dodano obsługę wyświetlacza LCD 2×16 I²C.
- Dodano wyświetlanie:
  - mocy grzałki (%),
  - stanu grzałki (ON/OFF),
  - liczby impulsów BurstFire,
  - częstotliwości sieci.
- Dodano mechanizm odświeżania LCD tylko po zmianie danych (`refreshDisplay()`).

### Testy

- Test H11AA1 zakończony powodzeniem.
- Stabilny odczyt:
  - HalfCycles = 100
  - Frequency = 50.0 Hz
  - Signal = TRUE
- Test sterowania MOC3083 zakończony powodzeniem.
- Test triaka z żarówką 100 W zakończony powodzeniem.
- Test algorytmu BurstFire 0–100% zakończony powodzeniem.
- Potwierdzono regulację prądu cęgami pomiarowymi (0,00–0,43 A).
- Uruchomiono i przetestowano wyświetlacz LCD 2×16 I²C.
- Zintegrowano moduł `DisplayManager` z projektem.

## v0.2.1

### Dodano

- Dodano moduł `ControlPanel`.
- Dodano trzy tryby pracy:
  - OFF,
  - AUTO,
  - MANUAL.
- Dodano obsługę trzech przycisków:
  - PLUS,
  - MODE,
  - MINUS.
- Dodano ręczną regulację mocy grzałki w zakresie 0–100% ze skokiem 10%.
- Dodano integrację modułu `ControlPanel` z `DisplayManager`.
- Dodano wyświetlanie aktualnego trybu pracy na LCD.
- Przebudowano układ wyświetlacza LCD do docelowego formatu:
  - Linia 1: tryb pracy, moc oraz stan grzałki.
  - Linia 2: BurstFire oraz częstotliwość sieci.
- Dodano integrację `ControlPanel` z modułem `BurstFire`.
- Dodano pierwszą wersję docelowego `main.cpp` integrującą:
  - Logger,
  - ZeroCross,
  - BurstFire,
  - HeaterOutput,
  - DisplayManager,
  - WiFiManager,
  - ControlPanel.

### Testy

- Test trzech przycisków ESP32 zakończony powodzeniem.
- Potwierdzono poprawną zmianę trybów:
  - OFF → AUTO → MANUAL.
- Potwierdzono poprawną regulację mocy:
  - 0–100%,
  - krok 10%.
- Test sterowania grzałką 1800 W zakończony powodzeniem.
- Potwierdzono poprawną współpracę:
  - ControlPanel,
  - BurstFire,
  - HeaterOutput,
  - DisplayManager.
- Potwierdzono prawidłową regulację mocy grzałki.
- Zmierzony prąd:
  - około 5 A przy 60%,
  - około 6,8 A przy 100%.
- Temperatura radiatora triaka:
  - około 23°C przy 60%,
  - około 30°C po zagotowaniu wody przy 100%.
- Nie stwierdzono niestabilności pracy układu podczas testów krótkotrwałych.

### Status wersji

✅ Wersja zakończona i zatwierdzona do dalszego rozwoju.