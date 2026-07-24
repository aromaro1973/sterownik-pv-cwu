# ESPNowManager

Modul obsluguje odbior telemetrii z falownika przez ESP-NOW.

## Funkcje

- inicjalizacja odbioru ESP-NOW,
- parsowanie pakietu `InverterPacket`,
- aktualizacja mocy: PV, falownik, bateria,
- diagnostyka lacza (licznik pakietow, straty, okres pakietu),
- wykrywanie utraty lacznosci (timeout 7 s).

## Uwagi

- callback ESP-NOW jest przekierowany przez wrapper C do instancji klasy,
- przy utracie lacznosci modul oznacza `connected=false` i resetuje synchronizacje ID pakietow.
