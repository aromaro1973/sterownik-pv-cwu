# Test Plan - Sterownik PV-CWU

Ten dokument opisuje szybki test funkcjonalny po wgraniu firmware.

## Warunki wstepne

- Firmware wgrany poprawnie.
- Nadajnik ESP-NOW (falownik) aktywny.
- Podlaczony LCD i przyciski PLUS/MODE/MINUS.
- Obciazenie grzalki podlaczone.

## Test 1 - Start po zaniku zasilania

Cel: potwierdzic, ze po restarcie sterownik startuje w AUTO z moca 0%.

Kroki:
1. Wylacz zasilanie sterownika.
2. Wlacz zasilanie sterownika.
3. Poczekaj na koniec splash screen (ok. 3 s).
4. Sprawdz ekran glowny.

Wynik oczekiwany:
- Tryb: AUTO.
- Moc startowa: 0%.
- Brak niekontrolowanego startu grzania przed telemetria.

## Test 2 - Failsafe przy zaniku telemetrii

Cel: potwierdzic, ze AUTO nie przechodzi do OFF, tylko zrzuca moc do 0%.

Kroki:
1. Doprowadz sterownik do pracy w AUTO z moca > 0%.
2. Wylacz nadajnik ESP-NOW albo zasymuluj brak nowych ramek.
3. Odczekaj min. 7 s.
4. Obserwuj ekran glowny i log UART.

Wynik oczekiwany:
- Tryb pozostaje AUTO.
- Moc spada do 0%.
- W logu pojawia sie komunikat failsafe 7s.

## Test 3 - Powrot telemetrii po failsafe

Cel: potwierdzic automatyczne wznowienie regulacji AUTO.

Kroki:
1. Bedac w stanie z Testu 2, przywroc nadajnik ESP-NOW.
2. Poczekaj na nowe ramki telemetryczne.
3. Obserwuj ekran i log.

Wynik oczekiwany:
- Pojawia sie log o powrocie telemetrii.
- Regulator AUTO sam wraca do wyliczania mocy.
- Nie jest wymagane klikanie MODE.

## Test 4 - MANUAL setup i zatwierdzenie

Cel: potwierdzic tryb dwuetapowy MANUAL (SET -> RUN).

Kroki:
1. Na ekranie glownym klikaj MODE: OFF -> AUTO -> MANUAL SET.
2. W MANUAL SET zmieniaj moc przyciskami PLUS/MINUS.
3. Sprawdz, czy grzalka nie startuje przed zatwierdzeniem.
4. Kliknij MODE, aby zatwierdzic i przejsc do MANUAL RUN.

Wynik oczekiwany:
- W MANUAL SET mozna tylko ustawic moc.
- Przed zatwierdzeniem wyjscie triaka pozostaje 0%.
- Po zatwierdzeniu moc jest utrzymywana na ustawionej wartosci.

## Test 5 - Nawigacja INFO/CONFIG

Cel: potwierdzic mapowanie ekranow i zapisywanie nastaw.

Kroki:
1. Z ekranu glownego kliknij PLUS i przejdz wszystkie ekrany INFO przez MODE.
2. Z ekranu glownego kliknij MINUS i przejdz wszystkie ekrany CONFIG przez MODE.
3. Na ostatnim ekranie CONFIG zatwierdz MODE (zapis).
4. Wykonaj restart zasilania.

Wynik oczekiwany:
- INFO: 7 ekranow zgodnie z mapa.
- CONFIG: 4 ekrany zgodnie z mapa.
- Po restarcie wartosci CONFIG sa zachowane (NVS).

## Test 6 - Polityka mocy 47/100

Cel: potwierdzic ograniczenie 48..99% do 47% oraz histereze 100%.

Kroki:
1. Wymus zadanie mocy powyzej 47%, ale ponizej 100%.
2. Sprawdz wykonana moc (applied power) w logu/diagnostyce.
3. Wymus 100%, a potem obniz zadanie do zakresu 80..99%.
4. Obniz dalej ponizej 80%.

Wynik oczekiwany:
- Dla 48..99% wykonanie = 47% (o ile nie jest zalaczona pelna fala).
- Po wejsciu na 100% utrzymuje sie pelna fala przy zadaniu >= 80%.
- Zejscie ponizej 80% odlacza latch i wraca do sterowania fazowego.

## Szybka checklista koncowa

- [ ] Start po restarcie: AUTO, 0%
- [ ] Failsafe 7 s: AUTO zostaje, moc 0%
- [ ] Powrot telemetrii: AUTO samo wraca do pracy
- [ ] MANUAL SET: regulacja bez podania mocy
- [ ] MANUAL RUN: stabilna moc po zatwierdzeniu
- [ ] INFO/CONFIG: poprawna nawigacja i zapis NVS
- [ ] Polityka 47/100 i histereza dzialaja poprawnie
