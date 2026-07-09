# Sterownik Nadwyżki PV

## Autor

Arkadiusz Marek

Projekt wykonywany przy współpracy z ChatGPT.

---

## Cel projektu

Sterownik nadwyżki energii z instalacji fotowoltaicznej oparty o ESP32.

Projekt przeznaczony jest do płynnego sterowania grzałką CWU w instalacji off-grid poprzez wykorzystanie nadwyżki energii z paneli fotowoltaicznych.

Projekt rozwijany jest modułowo. Każdy moduł jest projektowany, testowany i zatwierdzany oddzielnie, a następnie integrowany z pozostałymi elementami systemu.

---

## Funkcje

- płynna regulacja mocy grzałki CWU
- detekcja przejścia przez zero (H11AA1)
- sterowanie triakiem przez MOC3083
- algorytm BurstFire
- ręczny panel sterowania (OFF / AUTO / MANUAL)
- wyświetlacz LCD 2×16
- komunikacja WiFi
- integracja z Home Assistant
- moduł Guardian zabezpieczający pracę falownika
- komunikacja ESP-NOW

---

## Wersja

**v0.2.1**

---

## Zasady wersjonowania

- **v0.x.x** – rozwój projektu oraz testy.
- **v1.0.0** – pierwsza stabilna wersja po zakończeniu testów.

---

# Status projektu

## Moduły ukończone

- ✅ PlatformIO
- ✅ GitHub
- ✅ Struktura projektu
- ✅ Config
- ✅ Logger
- ✅ ZeroCross V2
- ✅ HeaterOutput
- ✅ BurstFire
- ✅ DisplayManager
- ✅ WiFiManager
- ✅ ControlPanel

## Moduły w trakcie

- ⏳ AutoController
- ⏳ Guardian
- ⏳ ESPNowManager
- ⏳ Home Assistant

---

## Sprzęt

- ESP32 DevKit V1
- H11AA1
- MOC3083
- Triak AKW33
- LCD 2×16 I²C (0x27)

---

## Środowisko

- PlatformIO
- Visual Studio Code
- Git
- GitHub

---

## Aktualny stan

### Zakończone testy

- ✅ Logger
- ✅ ZeroCross V2
- ✅ Detekcja przejścia przez zero H11AA1
- ✅ Sterowanie MOC3083
- ✅ Test triaka z żarówką 100 W
- ✅ Test BurstFire 0–100%
- ✅ Pomiar prądu cęgami
- ✅ Uruchomienie LCD 2×16
- ✅ Integracja DisplayManager
- ✅ Integracja ControlPanel
- ✅ Tryby OFF / AUTO / MANUAL
- ✅ Regulacja mocy 0–100% z przycisków
- ✅ Test z grzałką 1800 W

---

## Wyniki testów sprzętowych

### Żarówka 100 W

- poprawne sterowanie triakiem
- poprawna praca BurstFire
- regulacja mocy 0–100%

### Grzałka CWU 1800 W

- poprawna regulacja mocy
- około 5 A przy 60%
- około 6,8 A przy 100%
- temperatura radiatora około 23°C przy 60%
- temperatura radiatora około 30°C po zagotowaniu wody przy 100%
- układ pracował stabilnie

---

## Kolejne etapy projektu

- dokumentacja projektu
- panel WiFi
- integracja z Home Assistant
- AutoController
- Guardian
- pomiar temperatury radiatora
- testy długotrwałe
- pomiar temperatury CWU

---

## Architektura projektu

Każdy moduł projektu jest rozwijany i testowany niezależnie.

Obecne moduły:

- Config
- Logger
- ZeroCross
- HeaterOutput
- BurstFire
- DisplayManager
- ControlPanel
- WiFiManager

---

## Aktualna architektura sterownika

```
ControlPanel
        │
        ▼
AutoController
        │
        ▼
Guardian
        │
        ▼
BurstFire
        │
        ▼
HeaterOutput
        │
        ▼
Triak
        │
        ▼
Grzałka CWU
```

---

## Status

Projekt znajduje się w fazie aktywnego rozwoju.

Każdy moduł jest projektowany, testowany oraz zatwierdzany oddzielnie, a następnie integrowany z pozostałymi elementami systemu.

Po zakończeniu wszystkich testów planowane jest wydanie pierwszej stabilnej wersji **v1.0.0**.

## Licencja

Projekt rozwijany hobbystycznie.

© Arkadiusz Marek
