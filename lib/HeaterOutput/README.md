# HeaterOutput

## Zadanie modułu

Moduł `HeaterOutput` odpowiada za bezpośrednie sterowanie wyjściem ESP32 podłączonym do optotriaka MOC3083.

Jest jedynym modułem w projekcie, który steruje fizycznym wyjściem odpowiedzialnym za załączanie grzałki.

---

## Funkcje

- inicjalizacja wyjścia sterującego
- włączenie wyjścia
- wyłączenie wyjścia
- odczyt aktualnego stanu wyjścia

---

## Wejścia

Moduł nie pobiera danych bezpośrednio.

Sterowanie odbywa się poprzez wywołanie metod:

```cpp
on();
off();
```

---

## Wyjścia

Moduł steruje pinem:

```cpp
PIN_TRIAC
```

który podłączony jest do optotriaka MOC3083.

---

## Współpraca z modułami

```
BurstFire
     │
     ▼
HeaterOutput
     │
     ▼
MOC3083
     │
     ▼
Triak
     │
     ▼
Grzałka
```

---

## Moduł nie odpowiada za

- regulację mocy,
- algorytm BurstFire,
- detekcję przejścia przez zero,
- obsługę przycisków,
- wyświetlacz LCD,
- komunikację WiFi,
- automatykę sterownika.

---

## Testy

Moduł został sprawdzony podczas testów:

- sterowanie MOC3083,
- test triaka z żarówką 100 W,
- test grzałki 1800 W,
- poprawna współpraca z modułem BurstFire,
- poprawna współpraca z modułem ZeroCross.

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