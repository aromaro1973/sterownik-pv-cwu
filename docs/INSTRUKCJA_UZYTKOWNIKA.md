# Instrukcja Uzytkownika Koncowego

Wersja dokumentu: 1.0  
Dla firmware: aktualna wersja projektu Sterownik PV-CWU

## 1. Co robi sterownik

Sterownik PV-CWU automatycznie kieruje nadwyzke energii z instalacji off-grid do grzalki CWU.

Glowny cel pracy:
- utrzymac stabilna prace falownika,
- ograniczyc niepotrzebne rozladowanie baterii,
- wykorzystac dostepna nadwyzke do grzania,
- bezpiecznie reagowac na zanik telemetrii.

## 2. Tryby pracy

Sterownik pracuje w trzech trybach:
1. OFF: grzalka wylaczona.
2. AUTO: moc liczona automatycznie przez algorytm.
3. MANUAL: moc ustawiana recznie przez uzytkownika.

Uwaga:
- Po wlaczeniu zasilania sterownik startuje w AUTO z moca 0%.
- Przy zaniku telemetrii AUTO pozostaje aktywne, ale moc jest zrzucana do 0%.
- Po powrocie telemetrii AUTO samoczynnie wraca do regulacji.

## 3. Moduly sterownika i ich zadania

1. `main.cpp`
- Rola: glowna logika i maszyna stanow.
- Odpowiada za przelaczanie ekranow, tryby pracy i reakcje failsafe.

2. `ESPNowManager`
- Rola: odbior danych telemetrycznych z falownika (ESP-NOW).
- Dostarcza m.in. moc falownika, moc PV, moc baterii i status lacza.

3. `AutoController`
- Rola: wylicza zadana moc grzalki w trybie AUTO.
- Dziala w watach, potem przelicza wynik na procent mocy.

4. `PhaseController`
- Rola: wykonuje fizyczne sterowanie triakiem.
- Stosuje polityke mocy 0..47% (faza) oraz 100% (pelna fala).

5. `ZeroCross`
- Rola: wykrywa przejscia sieci przez zero.
- Synchronizuje moment wyzwalania triaka.

6. `Guardian`
- Rola: przechowuje parametry konfiguracyjne i zapisuje je w NVS.
- Udostepnia limity dla logiki AUTO.

7. `DisplayManager`
- Rola: obsluga LCD 16x2.
- Wyswietla ekran glowny, ekrany INFO i ekrany CONFIG.

8. `ControlPanel`
- Rola: obsluga przyciskow PLUS, MODE, MINUS.
- Wykrywa klik i dlugie przytrzymanie MODE.

9. `Logger`, `Utils`, `Config`
- Rola: narzedzia pomocnicze, stale projektu i diagnostyka.

## 4. Zaleznosci miedzy modulami

Przeplyw danych i sterowania:
1. `ESPNowManager` odbiera telemetrie z falownika.
2. `main.cpp` przekazuje dane do `AutoController`.
3. `AutoController` wylicza zadana moc.
4. `main.cpp` podaje zadanie do `PhaseController`.
5. `ZeroCross` synchronizuje wyzwalanie triaka w `PhaseController`.
6. `DisplayManager` pokazuje stan pracy i dane diagnostyczne.
7. `Guardian` przechowuje i odtwarza nastawy dla `main.cpp` i `AutoController`.

## 5. Obsluga panelu i wyswietlacza

Przyciski:
- PLUS
- MODE
- MINUS

## 5.1 Ekran glowny

Na ekranie glownym widoczny jest:
- aktualny tryb pracy,
- stan grzalki,
- zadana/realna moc (w %).

W MANUAL SET na dole pojawia sie napis `SET`.
To znaczy, ze moc jest ustawiana, ale nie zostala jeszcze zatwierdzona do pracy.

## 5.2 Dzialanie przyciskow na ekranie glownym

1. Krotki MODE:
- OFF -> AUTO
- AUTO -> MANUAL SET
- MANUAL SET -> MANUAL RUN (zatwierdzenie)
- MANUAL RUN -> OFF

2. Dlugie MODE (ok. 2 s):
- szybkie przejscie do OFF.

3. PLUS:
- wejscie do ekranow INFO (gdy nie jestes w MANUAL SET).

4. MINUS:
- wejscie do ekranow CONFIG (gdy nie jestes w MANUAL SET).

## 5.3 MANUAL SET i MANUAL RUN

1. Wejdz MODE do MANUAL SET.
2. Ustaw moc PLUS/MINUS co 5%.
3. Zatwierdz MODE.
4. Po zatwierdzeniu sterownik przechodzi do MANUAL RUN i utrzymuje zadana moc.

Zakres mocy recznej:
- 0% do 100%
- krok regulacji: 5%

## 5.4 Ekrany INFO

Kolejnosc ekranow INFO:
1. ESP-NOW RADIO
2. ZEROCROSS
3. PHASE CTRL
4. AUTOCONTROL
5. MOC GRZANIA GRZAL
6. MOC FALOWNIKA
7. MOC BATERII

Nawigacja:
- MODE przechodzi do kolejnego ekranu.
- Po 7 ekranie MODE wraca do ekranu glownego.

## 5.5 Ekrany CONFIG

Kolejnosc ekranow CONFIG:
1. MOC MAX FALOWNIK
2. MAX ROZLAD. BATER.
3. CZAS ZWLOKA PV
4. MOC GRZALKI

Nawigacja:
- PLUS/MINUS zmienia wartosc na biezacym ekranie.
- MODE przechodzi dalej.
- Na ostatnim ekranie MODE zapisuje ustawienia do pamieci i wraca do ekranu glownego.

## 6. Dane konfiguracyjne: zakresy i znaczenie

1. MOC MAX FALOWNIK
- Zakres: 100 W do 4000 W
- Krok: 100 W
- Znaczenie: gorny limit obciazenia falownika uzywany przez algorytm AUTO.

2. MAX ROZLAD. BATER.
- Zakres: 50 W do 2000 W
- Krok: 50 W
- Znaczenie: dopuszczalne rozladowanie baterii na potrzeby grzania CWU.

3. CZAS ZWLOKA PV
- Zakres: 100 ms do 5000 ms
- Krok: 100 ms
- Znaczenie: czas odczekania przed kolejnym podbiciem mocy w AUTO.

4. MOC GRZALKI
- Zakres: 50 W do 4000 W
- Krok: 50 W
- Znaczenie: moc znamionowa grzalki do prawidlowego przeliczania regulacji.

## 7. Dodatkowe informacje techniczne

1. Polityka wykonania mocy triaka:
- 0..47%: sterowanie fazowe,
- 48..99%: ograniczenie do 47%,
- 100%: pelna fala.

2. Po wejsciu na 100% aktywna jest histereza pelnej fali.
Pelna fala utrzymuje sie do czasu spadku komendy ponizej 80%.

3. Zapis ustawien jest trwaly (NVS), wiec po restarcie urzadzenie pamieta konfiguracje.

## 8. Szybki start dla uzytkownika

1. Wlacz zasilanie.
2. Poczekaj na koniec ekranu startowego.
3. Sterownik przejdzie do AUTO (0%).
4. Aby ustawic reczna moc, klikaj MODE do MANUAL SET.
5. Ustaw moc PLUS/MINUS i zatwierdz MODE.
6. Aby wejsc do konfiguracji, z ekranu glownego kliknij MINUS.
