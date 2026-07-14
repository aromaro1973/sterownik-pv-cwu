DOKUMENTACJA TECHNICZNA PROJEKTU
SYSTEM ZARZĄDZANIA ENERGIĄ (EMS): STEROWNIK NADWYŻKI PV (OFF-GRID)
Wersja dokumentu: 1.0

Autor projektu: Arkadiusz Marek

Główny Architekt AI / Współtwórca: Gemini (Sztuczna Inteligencja)

1. Założenia Projektowe i Koncepcja Systemu
Głównym celem systemu EMS (Energy Management System) jest bezprzewodowy odbiór danych telemetrycznych z falownika off-grid oraz precyzyjne, płynne przekierowanie nadwyżek energii elektrycznej z paneli fotowoltaicznych (PV) bezpośrednio do obciążenia rezystancyjnego (grzałka CWU o mocy nominalnej 2000 W).

Główne cele konstrukcyjne:
Bezpieczny Start (Failsafe Start): Sterownik po włączeniu zasilania lub restarcie zawsze inicjalizuje się w trybie bezpiecznym (WorkMode::OFF) oraz z mocą grzałki ustawioną na 0%.

Precyzyjne i Bezpieczne Sterowanie Triakiem: Zaimplementowanie sterowania fazowego z programowo i sprzętowo wydzielonym "pasmem zabronionym" (41%−94%), chroniącym falownik off-grid przed generowaniem szkodliwych harmonicznych i destabilizacją pracy stopnia wyjściowego.

Zaawansowana Diagnostyka i Watchdog Komunikacyjny: Wykrywanie przerw w transmisji radiowej ESP-NOW oraz blokowanie pracy przy wykryciu "zamrożenia" (braku zmian) danych telemetrycznych.

Wielordzeniowość (Multi-core) i Optymalizacja RAM: Odciążenie głównego rdzenia ESP32 poprzez obsługę zdarzeń krytycznych czasowo (Zero-Cross, wyzwalanie triaka) przy użyciu przerwań sprzętowych (IRAM_ATTR) oraz optymalizacja zużycia sterty (brak dynamicznych alokacji ciągów znakowych dzięki __FlashStringHelper i const char*).

2. Architektura i Zależności Modułowe
System EMS opiera się na architekturze modułowej. Poniższy diagram przedstawia przepływ danych, zdarzeń i sygnałów sterujących pomiędzy poszczególnymi klasami oprogramowania:

                  +-----------------------+
                  |  Odbiornik ESP-NOW   | <--- (Komunikacja Radiowa)
                  |    (ESPNowManager)    |
                  +-----------+-----------+
                              |
                              | Dane telemetryczne (InverterPacket - 10B)
                              v
                  +-----------------------+      Weryfikacja
                  |     Pętla EMS         | <====================> +-----------------+
                  |      (Main /          |     Stan blokady       |    Guardian     |
                  |   AutoController)     |                        | (Zabezpieczenia)|
                  +-----------+-----------+                        +-----------------+
                              |
                              | Żądana moc [%]
                              v
                  +-----------------------+
                  |    PhaseController    |
                  | (Przeliczanie kata    |
                  |   otwarcia triaka)    |
                  +-----------+-----------+
                              |
                              | Opóźnienie czasowe [us]
                              v
+------------------+      Taktowanie     +-----------------------+      Sygnał        +---------------+
| Detektor Zera AC | ==================> |      ZeroCross        | =================> | Fizyczny Pin  |
|  (PIN_ZERO_CROSS)|                     | (Obsługa przerwania)  |  Wyzwalający       |  (PIN_TRIAC)  |
+------------------+                     +-----------------------+                    +---------------+
                                                     ||
                                                     || Rejestracja statystyk (zcCounter, triggerCounter)
                                                     v
                                         +-----------------------+
                                         |    Utils / Logger     |
                                         |    (Narzędzia)        |
                                         +-----------------------+
3. Przegląd i Specyfikacja Modułów
3.1. main.cpp (Nadrzędny Koordynator)
Rola: Integruje wszystkie peryferia, realizuje główną maszynę stanów interfejsu użytkownika (Splash Screen → Ekrany Pracy → Menu Serwisowe), synchronizuje odczyty wejść z fizycznym wysterowaniem triaka w oparciu o detekcję przejść przez zero.

Główne zabezpieczenie: Blokuje i zeruje moc grzałki w trybie Menu Serwisowego. Wykrywa zamrożenie danych telemetrycznych oraz brak zasięgu radiowego, wykonując natychmiastowy, awaryjny zrzut do bezpiecznego trybu manualnego o mocy 0%.

3.2. ZeroCross
Rola: Detektor przejścia napięcia sieci przez zero.

Mechanika: Rejestruje przerwania sprzętowe na pinie PIN_ZERO_CROSS z filtrem zakłóceń (5 ms). Oblicza w czasie rzeczywistym częstotliwość sieci [Hz] oraz czasowo weryfikuje obecność napięcia zmiennego (Hardware Watchdog).

3.3. PhaseController
Rola: Sterownik kąta otwarcia triaka.

Mechanika: Dokonuje konwersji żądanej mocy (0%−100%) na opóźnienie wyzwolenia bramki triaka w mikrosekundach (μs). Implementuje sprzętowe limity bezpieczeństwa: praca fazowa tylko w zakresie 1%−40%, pasmo zabronione 41%−94%, oraz przejście na pełną sinusoidę przy wartościach ≥95%.

3.4. ESPNowManager
Rola: Bezprzewodowy odbiór paczek radiowych z nadajnika.

Mechanika: Odbiera upakowaną strukturę telemetryczną InverterPacket o stałym rozmiarze 10 bajtów. Prowadzi statystyki QoS (Quality of Service) na podstawie analizy monotonicznie rosnącego indeksu ramek packetId, obliczając liczbę utraconych pakietów i jitter (interwał odbioru).

3.5. Guardian
Rola: Bezpiecznik nadrzędny (Software Interlock).

Mechanika: Niezależnie nadzoruje limity fizyczne i prądowe. W przypadku naruszenia maksymalnej mocy dopuszczalnej (maxPower) lub przekroczenia maksymalnego przyrostu obciążenia w czasie (powerStep), natychmiastowo aktywuje blokadę, odcinając wysterowanie grzałki.

3.6. Utils
Rola: Narzędzia globalne i liczniki diagnostyczne.

Mechanika: Odpowiada za bezpieczny i nieblokujący pomiar upływu czasu (odporny na przepełnienie millis()), ograniczanie wartości (clamp) oraz utrzymywanie statystyk przejść przez zero i wyzwoleń triaka (volatile zmienne współdzielone).

4. Strategia Przyszłego Rozwoju Projektu
System został zaprojektowany z myślą o łatwej rozbudowie i integracji z nowoczesnymi systemami automatyki budynkowej (Smart Home). Planowane są dwa kluczowe kierunki rozwoju:

4.1. Integracja z Home Assistant (HA) i protokół MQTT
W celu pełnej wizualizacji danych w panelu Home Assistant planuje się wdrożenie dwukierunkowej komunikacji sieciowej:

Publikowanie Danych (State Topic): Sterownik będzie wysyłał na serwer MQTT informacje o aktualnej temperaturze, mocy wysterowania grzałki, częstotliwości sieci oraz statystykach jakości sygnału radiowego (QoS).

Sterowanie i Zmiana Nastaw (Command Topic): Home Assistant zyska możliwość zdalnej zmiany trybu pracy (np. wymuszenie grzania w trybie Manual przed planowaną kąpielą) oraz zmiany krytycznych nastaw parametrów zabezpieczeń modułu Guardian bezpośrednio z poziomu dashboardu Lovelace.

Separacja Radiowa: Praca Wi-Fi w trybie dwufunkcyjnym (Dual Mode: ESP-NOW + Wi-Fi dla MQTT) zostanie zoptymalizowana na dedykowanym kanale, aby transmisja TCP/IP do brokera MQTT nie kolidowała z priorytetowym, krytycznym czasowo odbiorem ramek ESP-NOW.

4.2. Modernizacja i Dopracowanie Nadajnika ESP-NOW
Nadajnik telemetryczny, zlokalizowany bezpośrednio przy falowniku off-grid, zostanie zmodernizowany o następujące funkcjonalności:

Aktywne Potwierdzenia (ACK) i Retransmisja: Dodanie mechanizmu dwukierunkowej weryfikacji pakietów. W przypadku braku potwierdzenia odbioru ze strony odbiornika, nadajnik podejmie próbę powtórzenia wysyłki, co poprawi odporność na chwilowe zakłócenia w pasmie 2.4 GHz.

Dynamiczne Dopasowanie Mocy Radiowej: Nadajnik na podstawie informacji zwrotnej o liczbie zagubionych pakietów (dane z licznika getLostPackets() przesyłane z powrotem) będzie mógł automatycznie zwiększać moc nadawczą (Tx power), redukując ją do minimum przy doskonałym zasięgu w celu oszczędności energii i eliminacji smogu elektromagnetycznego.

Wielofunkcyjny Sensor Prądu: Bezpośrednie podpięcie pod szyny falownika dedykowanego układu pomiarowego (np. przekładnika AC PZEM-004T lub czujników SCT) w celu jeszcze dokładniejszego wykrywania rzeczywistego kierunku przepływu energii w punkcie przyłączeniowym.