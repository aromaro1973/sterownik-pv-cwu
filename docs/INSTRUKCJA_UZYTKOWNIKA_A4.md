# Sterownik PV-CWU
## Instrukcja Uzytkownika (Wersja do druku A4)

Wersja dokumentu: 1.0  
Przeznaczenie: szybka obsluga i konfiguracja przez uzytkownika koncowego

---

## 1. Co robi sterownik

Sterownik automatycznie kieruje nadwyzke energii do grzalki CWU.

Cele pracy:
- stabilna praca falownika,
- ograniczenie rozladowania baterii,
- wykorzystanie nadwyzki energii,
- bezpieczna reakcja na brak telemetrii.

---

## 2. Tryby pracy

1. OFF
- Grzalka wylaczona.

2. AUTO
- Moc liczona automatycznie.
- Po wlaczeniu zasilania sterownik startuje w AUTO z moca 0%.

3. MANUAL
- Moc ustawiana recznie przez uzytkownika.
- Praca 2-etapowa: SET (ustawianie) -> RUN (zatwierdzona praca).

---

## 3. Failsafe (brak telemetrii)

Jesli przez ok. 7 s nie ma swiezych danych:
- tryb AUTO pozostaje aktywny,
- moc spada do 0%,
- po powrocie telemetrii AUTO samo wraca do regulacji.

---

## 4. Panel sterowania

Przyciski:
- PLUS
- MODE
- MINUS

Na ekranie glownym:
1. Krotki MODE:
- OFF -> AUTO
- AUTO -> MANUAL SET
- MANUAL SET -> MANUAL RUN
- MANUAL RUN -> OFF

2. Dlugie MODE (ok. 2 s):
- szybkie przejscie do OFF.

3. PLUS:
- wejscie do ekranow INFO (poza MANUAL SET).

4. MINUS:
- wejscie do ekranow CONFIG (poza MANUAL SET).

---

## 5. Ekrany LCD

### Ekran glowny
- pokazuje tryb pracy,
- stan grzalki,
- moc w procentach.

W MANUAL SET pojawia sie napis SET.

Jak czytac ekran glowny:
- TRYB OFF: wyjscie grzalki jest wylaczone.
- TRYB AUTO: moc jest liczona dynamicznie z danych falownika i baterii.
- TRYB MANUAL + SET: ustawiasz moc, ale nie jest jeszcze zatwierdzona do pracy.
- TRYB MANUAL bez SET: grzalka pracuje ze stala, zatwierdzona moca.

### Ekrany INFO (MODE = dalej)
1. ESP-NOW RADIO
2. ZEROCROSS
3. PHASE CTRL
4. AUTOCONTROL
5. MOC GRZANIA GRZAL
6. MOC FALOWNIKA
7. MOC BATERII
8. MOC PRODUKCJI PV

Po ekranie 8: powrot do ekranu glownego.

Co pokazuje kazdy ekran INFO i z czym jest zwiazany:

1. ESP-NOW RADIO
- Co pokazuje: status lacza RADIO:OK/OFF i okres ostatnich ramek w ms.
- Zwiazek z systemem: bez swiezych ramek AUTO nie moze liczyc mocy i przechodzi w tryb bezpieczny 0%.

2. ZEROCROSS
- Co pokazuje: liczbe przejsc przez zero (ZC) i czestotliwosc sieci (Hz).
- Zwiazek z systemem: to podstawa synchronizacji wyzwalania triaka.

3. PHASE CTRL
- Co pokazuje: liczbe wyzwolen triaka (TR) i odniesienie do ZC.
- Zwiazek z systemem: pozwala ocenic, czy wykonanie mocy jest stabilne i zsynchronizowane.

4. AUTOCONTROL
- Co pokazuje: usredniona moc sterowania (AVG PWR).
- Zwiazek z systemem: pokazuje jak regulator AUTO dostraja moc w czasie.

5. MOC GRZANIA GRZAL
- Co pokazuje: usredniony poziom grzania grzalki (AVG).
- Zwiazek z systemem: praktyczna informacja o realnym wykorzystaniu grzalki.

6. MOC FALOWNIKA
- Co pokazuje: aktualna moc wyjsciowa falownika w W.
- Zwiazek z systemem: kluczowa wielkosc do pilnowania limitu falownika.

7. MOC BATERII
- Co pokazuje: kierunek i wartosc mocy baterii (LAD/ROZ).
- Zwiazek z systemem: dodatnia rozlad (ROZ), ujemna ladowanie (LAD); AUTO pilnuje, by nie przekroczyc limitu rozladowania.

8. MOC PRODUKCJI PV
- Co pokazuje: aktualna produkcje PV w W.
- Zwiazek z systemem: pomocnicza informacja diagnostyczna o dostepnej energii.

### Ekrany CONFIG (PLUS/MINUS zmiana, MODE dalej)
1. MOC MAX FALOWNIK
2. MAX ROZLAD. BATER.
3. CZAS ZWLOKA PV
4. MOC GRZALKI

Na ostatnim ekranie MODE zapisuje ustawienia i wraca do MAIN.

---

## 6. Ustawienia konfiguracyjne

1. MOC MAX FALOWNIK
- Zakres: 100 do 4000 W
- Krok: 100 W
- Do czego sluzy: limit obciazenia falownika.
- Wplyw na prace:
	- wyzsza wartosc: AUTO moze dluzej podbijac moc grzalki,
	- nizsza wartosc: szybsza redukcja mocy grzalki i bardziej zachowawcza praca.

2. MAX ROZLAD. BATER.
- Zakres: 50 do 2000 W
- Krok: 50 W
- Do czego sluzy: dopuszczalne rozladowanie baterii na CWU.
- Wplyw na prace:
	- wyzsza wartosc: wieksza zgoda na pobor energii z baterii,
	- nizsza wartosc: priorytet ochrony baterii, czestsze zbijanie mocy grzalki.

3. CZAS ZWLOKA PV
- Zakres: 100 do 5000 ms
- Krok: 100 ms
- Do czego sluzy: opoznienie przed kolejnym podbiciem mocy w AUTO.
- Wplyw na prace:
	- krotki czas: szybsze reakcje, ale wieksza nerwowosc regulacji,
	- dlugi czas: spokojniejsza i stabilniejsza regulacja, wolniejsze podbijanie mocy.

4. MOC GRZALKI
- Zakres: 50 do 4000 W
- Krok: 50 W
- Do czego sluzy: moc znamionowa grzalki do poprawnych przeliczen.
- Wplyw na prace:
	- wartosc zawyzona: regulator moze zawyzac mozliwa moc i reagowac mniej precyzyjnie,
	- wartosc zanizona: regulator bedzie zbyt ostrozny i ograniczy wykorzystanie nadwyzki.

Wazne:
- Parametry dzialaja razem i trzeba je dobierac jako zestaw.
- Zmiana jednego parametru moze zmienic zachowanie AUTO na kilku ekranach INFO.

Ustawienia sa zapisywane w pamieci trwalej (NVS).

---

## 7. Informacje techniczne (skrot)

- 0..47%: sterowanie fazowe.
- 48..99%: ograniczenie do 47%.
- 100%: pelna fala.
- Histereza pelnej fali: po wejsciu na 100% utrzymanie do spadku komendy ponizej 80%.

---

## 8. Szybki start (krotko)

1. Wlacz zasilanie.
2. Poczekaj na koniec ekranu startowego.
3. Sterownik przejdzie do AUTO (0%).
4. Dla pracy recznej: MODE -> MANUAL SET, ustaw PLUS/MINUS, MODE zatwierdz.
5. Konfiguracja: z ekranu glownego kliknij MINUS.

---

## 9. Checklista odbioru

- [ ] Start po restarcie: AUTO, 0%
- [ ] Failsafe: AUTO zostaje, moc 0%
- [ ] Powrot telemetrii: AUTO wraca samo
- [ ] MANUAL SET dziala i nie uruchamia grzania przed zatwierdzeniem
- [ ] INFO i CONFIG przewijaja sie poprawnie
- [ ] Zapis konfiguracji dziala po restarcie
