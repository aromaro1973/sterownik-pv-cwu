Moduł: DisplayManager (v1.0)Moduł DisplayManager odpowiada za kompletną wizualizację stanu pracy sterownika oraz konfigurację ustawień serwisowych na alfanumerycznym ekranie LCD 16x2 z interfejsem $I^2C$. Projekt uwzględnia optymalizację transmisji danych w celu zminimalizowania obciążenia mikrokontrolera ESP32.💡 Koncepcja i optymalizacja renderowaniaKomunikacja z wyświetlaczami $I^2C$ bywa wąskim gardłem w systemach czasu rzeczywistego (wyzwalanie triaka wymaga mikrosekundowej precyzji). W związku z tym wdrożono następujące techniki:Ograniczenie częstotliwości odświeżania (Rate Limiting): Nawet jeśli stan systemu zmienia się dynamicznie, fizyczna aktualizacja ekranu odbywa się maksymalnie raz na 300 ms (now - m_lastRefreshTime >= 300). Zapobiega to migotaniu wyświetlacza i odciąża magistralę.Odświeżanie warunkowe (Dirty Flag): Ekran jest przerysowywany wyłącznie wtedy, gdy flaga m_refreshRequired ma wartość true (czyli zmieniono dane wejściowe, np. moc, tryb lub ekrany).Ciągły monitoring diagnostyczny: Ograniczenie 300 ms zostaje programowo pominięte w menu serwisowym (DisplayScreen::SERVICE), aby zapewnić natychmiastowy i płynny podgląd kluczowych parametrów sieciowych (częstotliwości i przejść przez zero).🖥️ Drzewo ekranów (Architektura Menu)System wyświetlacza posiada trzystopniową strukturę ekranów:[SPLASH SCREEN] (Uruchamianie urządzenia)
       │
       ├──► [MAIN SCREEN] (Ekrany pracy)
       │         ├──► Sub 0: Główny podgląd (Tryb, Moc %, Stan grzałki, Stan radia)
       │         ├──► Sub 1: Pomiary PV (Aktualna moc generowana)
       │         ├──► Sub 2: Inwerter (Aktualne obciążenie domu i falownika)
       │         └──► Sub 3: Akumulator (Pobór mocy, kierunek: ładowanie/rozładowanie)
       │
       └──► [SERVICE SCREEN] (Menu diagnostyczno-serwisowe)
                 ├──► Serwis 2: Status ZeroCross (Ilość detekcji ZC/s, częstotliwość Hz)
                 ├──► Serwis 3: Status PhaseController (Ilość rzeczywistych wyzwoleń triaka/s)
                 ├──► Serwis 4: Guardian - Max Inverter Power (Ustawienie limitu mocy falownika)
                 ├──► Serwis 5: Guardian - Delta Power (Ustawienie kroku regulacji mocy)
                 ├──► Serwis 6: ESP-NOW Radio (Jakość sygnału radiowego, średni czas odpowiedzi)
                 └──► Serwis 7: AutoController Status (Status działania pętli EMS)
🛠️ Opis interfejsu programistycznego (API)Metody publiczneDisplayManager()Opis: Konstruktor klasy. Ustala adres ekranu na 0x27 (wymiary 16x2), ustawia ekran startowy (SPLASH) oraz zeruje wszystkie zmienne pomocnicze i statystyki.void begin()Opis: Inicjalizuje magistralę $I^2C$ na pinach SDA: 21 oraz SCL: 22, uruchamia sterownik LCD, włącza podświetlenie i wyświetla Splash Screen.void update(const ESPNowManager& espNow)Opis: Główna metoda aktualizacyjna wywoływana w pętli loop(). Pilnuje limitu czasu odświeżania ($300\text{ ms}$) oraz decyduje, czy nadszedł moment na wyrenderowanie nowej klatki obrazu.void updateDiagnostics(uint32_t zc, uint32_t triggers)Opis: Przekazuje aktualne, sprzętowe statystyki przejść przez zero oraz wyzwoleń triaka zliczane na bieżąco przez przerwania mikrokontrolera.Metody nawigacyjnevoid setScreen(DisplayScreen screen) – Przełącza między głównymi trybami (SPLASH, MAIN, SERVICE).void setSubScreen(uint8_t subScreen) – Zmienia podstronę w menu głównym ($0-3$).void setServiceScreen(uint8_t serviceScreen) – Przełącza ekrany diagnostyczne w menu serwisowym ($2-7$).void forceRefresh() – Ignoruje liczniki czasu i wymusza natychmiastowe przerysowanie całego ekranu w następnym kroku pętli.📊 Wybrane wizualizacje ekranówEkran Główny (drawMainScreen)Wizualizuje aktualną nastawę, zmierzoną moc oraz łączność radiową:Plaintext+----------------+
|AUTO   35%    ON|
|RADIO: OK       |
+----------------+
Ekran Diagnostyki Przejścia przez Zero (drawZeroCrossScreen)Prezentuje liczbę wykrytych zboczy sieci $AC$ oraz precyzyjną częstotliwość:Plaintext+----------------+
|MOD: ZeroCrossOK|
|ZC/s:100 F:50.0H|
+----------------+
Ekran Statusu Triaka (drawPhaseManagerScreen)Umożliwia weryfikację, czy triak jest poprawnie wyzwalany w każdym półokresie:Plaintext+----------------+
|MOD: PhaseCtrACT|
|Trig/s:  40/100 |
+----------------+