# Logger

## Zadanie modułu

Moduł `Logger` odpowiada za wyświetlanie komunikatów diagnostycznych na porcie szeregowym.

Jego zadaniem jest ułatwienie uruchamiania, testowania oraz diagnostyki wszystkich modułów sterownika.

---

## Funkcje

- inicjalizacja portu szeregowego
- komunikaty DEBUG
- komunikaty INFO
- komunikaty WARNING
- komunikaty ERROR
- możliwość włączenia lub wyłączenia komunikatów DEBUG
- automatyczne dodawanie czasu od uruchomienia programu

---

## Wejścia

Moduł udostępnia metody:

```cpp
debug()
info()
warning()
error()
```

oraz

```cpp
setDebug()
```

---

## Wyjścia

Komunikaty wysyłane są na port szeregowy.

Przykład:

```text
[1250] [INFO ] Sterownik Nadwyzki PV
[1305] [INFO ] WiFi polaczone
[2100] [DEBUG] HalfCycles=100
```

---

## Współpraca z modułami

Logger może być wykorzystywany przez wszystkie moduły projektu.

```
              Logger
                 ▲
                 │
 ┌───────────────┼────────────────┐
 │               │                │
Config      ZeroCross      BurstFire
 │               │                │
 └───────────────┼────────────────┘
                 │
          pozostałe moduły
```

---

## Moduł nie odpowiada za

- sterowanie grzałką,
- regulację mocy,
- obsługę LCD,
- komunikację WiFi,
- Home Assistant,
- automatykę sterownika.

---

## Testy

Moduł został sprawdzony podczas testów:

- poprawna inicjalizacja portu szeregowego,
- poprawne wyświetlanie wszystkich poziomów logowania,
- poprawne odmierzanie czasu od uruchomienia programu,
- poprawna współpraca ze wszystkimi modułami projektu.

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