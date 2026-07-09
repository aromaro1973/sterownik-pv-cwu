# TODO – Sterownik Nadwyżki PV

## Wersja projektu

**v0.2.1**

---

# Etap 1 – Fundament projektu

- [x] PlatformIO
- [x] GitHub
- [x] Struktura projektu
- [x] Config
- [x] Logger
- [x] ZeroCross
- [x] HeaterOutput
- [x] BurstFire
- [x] DisplayManager
- [x] WiFiManager
- [x] ControlPanel

---

# Etap 2 – Testy sprzętu

- [x] Test H11AA1
- [x] Test MOC3083
- [x] Test triaka z żarówką 100 W
- [x] Test BurstFire 0–100%
- [x] Test LCD 2×16
- [x] Test trzech przycisków
- [x] Test grzałki 1800 W
- [ ] Test długotrwały grzałki

---

# Etap 3 – Dokumentacja

- [ ] Aktualizacja README
- [ ] Aktualizacja CHANGELOG
- [ ] Aktualizacja TODO

---

# Etap 4 – Panel WiFi

- [ ] Informacje o połączeniu WiFi
- [ ] Wyświetlanie adresu IP
- [ ] Status połączenia na LCD

---

# Etap 5 – Home Assistant

- [ ] MQTT
- [ ] Integracja z Home Assistant
- [ ] Sterowanie trybem pracy
- [ ] Sterowanie mocą
- [ ] Odczyt parametrów

---

# Etap 6 – AutoController

- [ ] Projekt modułu
- [ ] Regulacja według nadwyżki PV
- [ ] Sterowanie mocą grzałki
- [ ] Obsługa SOC akumulatora

---

# Etap 7 – Guardian

- [ ] Projekt modułu
- [ ] Ograniczenie mocy falownika
- [ ] Wyłączenie grzałki
- [ ] Obsługa alarmów

---

# Etap 8 – Monitorowanie temperatury

- [ ] Czujnik temperatury radiatora
- [ ] Alarm przegrzania
- [ ] Test długotrwały

---

# Etap 9 – CWU

- [ ] Przeniesienie sondy do zasobnika CWU
- [ ] Pomiar temperatury CWU
- [ ] Integracja z AutoController

---

# Etap 10 – Testy końcowe

- [ ] Test ciągły 24 h
- [ ] Test z instalacją PV
- [ ] Test bezpieczeństwa
- [ ] Wersja 1.0.0

## Pomysły na przyszłość

- [ ] Aktualizacja OTA
- [ ] Zapis ustawień do pamięci Flash
- [ ] Statystyki pracy grzałki
- [ ] Licznik energii oddanej do CWU
- [ ] Strona WWW sterownika