# ControlPanel

## Zadanie modułu

Moduł `ControlPanel` odpowiada za obsługę panelu sterowania użytkownika.

Umożliwia wybór trybu pracy sterownika oraz ręczną regulację mocy grzałki.

---

## Funkcje

- obsługa przycisku **MODE**
- obsługa przycisku **PLUS**
- obsługa przycisku **MINUS**
- wybór trybu:
  - OFF
  - AUTO
  - MANUAL
- regulacja mocy w trybie MANUAL
- zmiana mocy w krokach 10%

---

## Wejścia

- przycisk MODE
- przycisk PLUS
- przycisk MINUS

---

## Wyjścia

Moduł udostępnia:

```cpp
WorkMode getMode();
```

oraz

```cpp
uint8_t getManualPower();
```

---

## Współpraca z modułami

```
Użytkownik
     │
     ▼
ControlPanel
     │
     ├────────► DisplayManager
     │
     └────────► AutoController
                    │
                    ▼
                BurstFire
```

---

## Moduł nie odpowiada za

- sterowanie triakiem,
- algorytm BurstFire,
- sterowanie grzałką,
- zabezpieczenia Guardian,
- komunikację WiFi,
- integrację z Home Assistant.

---

## Aktualna logika pracy

### OFF

- grzałka wyłączona,
- moc 0%.

### AUTO

- moc ustawiana przez `AutoController`.

### MANUAL

- moc ustawiana przyciskami PLUS i MINUS,
- zakres 0–100%,
- krok regulacji 10%.

---

## Testy

Moduł został sprawdzony podczas testów:

- poprawna obsługa trzech przycisków,
- poprawna zmiana trybów:
  - OFF → AUTO → MANUAL,
- poprawna regulacja mocy:
  - 0–100%,
  - krok 10%,
- poprawna współpraca z modułem `DisplayManager`,
- poprawna współpraca z modułem `BurstFire`,
- poprawne sterowanie grzałką 1800 W.

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