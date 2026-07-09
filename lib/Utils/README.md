# Utils

## Zadanie modułu

Moduł `Utils` zawiera zbiór uniwersalnych funkcji pomocniczych wykorzystywanych przez różne moduły projektu.

Dzięki wydzieleniu tych funkcji do osobnej klasy unikamy powielania kodu oraz zwiększamy czytelność całego projektu.

---

## Funkcje

- ograniczenie wartości do zadanego zakresu (`clamp`)
- odmierzanie czasu (`elapsed`)
- konwersja wartości logicznej na tekst (`boolToString`)
- konwersja stanu na tekst ON/OFF (`onOff`)
- przeliczanie procentów na moc (`percentToPower`)

---

## Dostępne funkcje

```cpp
clamp()
elapsed()
boolToString()
onOff()
percentToPower()
```

---

## Współpraca z modułami

Moduł może być wykorzystywany przez wszystkie moduły projektu.

```
             Utils
                ▲
                │
 ┌──────────────┼──────────────┐
 │              │              │
Logger     ZeroCross     BurstFire
 │              │              │
 └──────────────┼──────────────┘
                │
        pozostałe moduły
```

---

## Moduł nie odpowiada za

- sterowanie grzałką,
- sterowanie triakiem,
- obsługę LCD,
- komunikację WiFi,
- automatykę sterownika,
- logowanie komunikatów.

---

## Testy

Sprawdzono poprawne działanie:

- funkcji `clamp()`,
- funkcji `elapsed()`,
- funkcji `boolToString()`,
- funkcji `onOff()`,
- funkcji `percentToPower()`.

Moduł wykorzystywany jest podczas testów wszystkich pozostałych modułów projektu.

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