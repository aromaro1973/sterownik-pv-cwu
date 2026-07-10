# # HAManager

## Zadanie modułu
Moduł `HAManager` odpowiada za pełną, dwukierunkową integrację sterownika z systemem automatyki domowej **Home Assistant** za pomocą protokołu **MQTT**. 

Pracuje asynchronicznie w tle na **CORE 1**. Wykorzystuje standard **MQTT Discovery**, eliminując potrzebę jakiejkolwiek konfiguracji ręcznej po stronie serwera Home Assistant.

---

## Funkcje
* **Auto-Discovery:** Automatyczne rejestrowanie sterownika jako fizycznego urządzenia w HA wraz z kompletem encji (sensory, suwaki, listy wyboru).
* **Sterowanie dwukierunkowe:** Zmiany trybów lub mocy w HA natychmiastowo nadpisują stan maszyny stanów w sterowniku, a fizyczne kliknięcia przycisków na obudowie natychmiast odświeżają suwaki w aplikacji HA.
* **Wskaźnik Dostępności (LWT):** Sprzętowy mechanizm informujący Home Assistant o nagłej utracie zasilania lub łączności przez sterownik (status `unavailable`).
* **Telemetria diagnostyczna:** Przesyłanie wskaźników jakości linku radiowego (QoS z ESP-NOW) bezpośrednio na wykresy analityczne.

---

## Sterowanie z Home Assistant
Z poziomu dashboardu HA użytkownik otrzymuje dostęp do:
* `select.tryb_pracy` — Wybór stanu systemu (`OFF`, `AUTO`, `MANUAL`).
* `number.moc_reczna` — Suwak $0\% - 100\%$ sterujący bezpośrednio wypełnieniem sinusoidy triaka w trybie ręcznym.

---

## Status i Wersja
* **Status:** 🟢 Klasa gotowa do testów sieciowych. Zgodna z PubSubClient.
* **Wersja:** 0.1
* **Autor:** Arkadiusz Marek