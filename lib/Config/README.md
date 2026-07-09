# Config

## Zadanie modułu

Moduł `Config` zawiera wszystkie stałe konfiguracyjne wykorzystywane przez projekt.

Dzięki temu wszystkie parametry sprzętowe i programowe znajdują się w jednym miejscu, co ułatwia rozwój oraz późniejszą konfigurację sterownika.

---

## Funkcje

- definicja pinów ESP32
- wersja programu
- nazwa urządzenia
- parametry sieci AC
- konfiguracja BurstFire
- zakres regulacji mocy
- konfiguracja portu szeregowego
- konfiguracja WiFi
- ustawienia debugowania

---

## Zawiera

- konfigurację pinów ESP32
- parametry sieci 50 Hz
- parametry BurstFire
- konfigurację WiFi
- wersję sprzętu
- wersję oprogramowania

---

## Nie odpowiada za

- obsługę sprzętu
- sterowanie grzałką
- algorytm BurstFire
- wyświetlacz LCD
- komunikację WiFi
- automatykę sterownika

---

## Współpraca z modułami

Moduł wykorzystywany jest przez wszystkie pozostałe moduły projektu.

```
                Config
                   │
       ┌───────────┼───────────┐
       │           │           │
    Logger     ZeroCross   BurstFire
       │           │           │
       └───────────┼───────────┘
                   │
           pozostałe moduły
```

---

## Testy

- sprawdzono poprawność definicji pinów
- sprawdzono konfigurację BurstFire
- sprawdzono konfigurację WiFi
- sprawdzono konfigurację wszystkich modułów

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