# Sterownik PV-CWU / EMS Off-Grid

Aktualna dokumentacja firmware dla sterownika grzalki CWU sterowanej z nadwyzek energii off-grid.

## Cel projektu

Sterownik odbiera telemetrie z falownika przez ESP-NOW i steruje grzalka tak, aby:
- maksymalnie wykorzystac nadwyzke,
- nie przeciazyc falownika,
- nie przekroczyc dopuszczalnego rozladowania baterii,
- reagowac bezpiecznie na zanik telemetrii.

## Tryby pracy

- `AUTO`: algorytm wylicza zadana moc na podstawie mocy falownika i baterii.
- `MANUAL`: tryb dwuetapowy:
	- etap ustawiania (`SET`) - PLUS/MINUS zmieniaja moc,
	- etap pracy - moc jest utrzymywana stale.
- `OFF`: grzalka wylaczona.

Po starcie zasilania sterownik uruchamia sie w `AUTO` z moca `0%`.

## Failsafe komunikacji

Failsafe (7 s) dziala globalnie:
- przy braku nowych ramek lub zamrozeniu telemetrii tryb `AUTO` pozostaje aktywny,
- moc jest zrzucana do `0%`,
- po powrocie telemetrii regulacja `AUTO` wznawia sie automatycznie.

## Zasada sterowania moca

- `0..47%`: sterowanie fazowe.
- `48..99%`: komenda ograniczana do `47%`.
- `100%`: pelna fala z histereza podtrzymania (latched full power) w `PhaseController`.

## Interfejs LCD i klawisze

Ekrany glowne i menu:
- `MAIN`: tryb i stan grzalki.
- `INFO` (7 ekranow): radio, zero-cross, triak, auto, moc grzania, moc falownika, moc baterii.
- `CONFIG` (4 ekrany):
	1. MOC MAX FALOWNIK
	2. MAX ROZLAD. BATER.
	3. CZAS ZWLOKA PV
	4. MOC GRZALKI

Mapowanie klawiszy na ekranie glownym:
- `MODE`: cykl OFF -> AUTO -> MANUAL SET -> MANUAL RUN -> OFF,
- `PLUS`: wejscie do INFO (poza MANUAL SET),
- `MINUS`: wejscie do CONFIG (poza MANUAL SET),
- `MODE` long (2 s): szybki zrzut do OFF.

## Rola modulow

- `src/main.cpp`: maszyna stanow i orkiestracja.
- `lib/ESPNowManager`: odbior telemetrii i diagnostyka ramek.
- `lib/AutoController`: regulator mocy w watach.
- `lib/PhaseController`: wykonanie triaka i polityka 47/100.
- `lib/ZeroCross`: detekcja przejsc przez zero i synchronizacja.
- `lib/Guardian`: parametry i zapis/odczyt NVS.
- `lib/DisplayManager`: renderowanie LCD.
- `lib/ControlPanel`: obsluga przyciskow.
- `lib/Logger`, `lib/Utils`, `lib/Config`: narzedzia i konfiguracja.

## Build

Zweryfikowane lokalnie:

`platformio run`

Wynik: `Exit Code: 0`.

## Testy funkcjonalne

Szczegolowy scenariusz testow po wgraniu firmware znajduje sie w pliku [docs/TEST_PLAN.md](docs/TEST_PLAN.md).

## Dokumenty dla uzytkownika

Instrukcja do druku dla uzytkownika koncowego znajduje sie w pliku [docs/INSTRUKCJA_UZYTKOWNIKA.md](docs/INSTRUKCJA_UZYTKOWNIKA.md).

Wersja skrocona pod wydruk A4 znajduje sie w pliku [docs/INSTRUKCJA_UZYTKOWNIKA_A4.md](docs/INSTRUKCJA_UZYTKOWNIKA_A4.md).