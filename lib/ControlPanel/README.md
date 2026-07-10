# ControlPanel
 Zadanie modułuModuł ControlPanel odpowiada za niskopoziomową obsługę lokalnego interfejsu użytkownika (fizycznej klawiatury trójprzyciskowej).Działa w warstwie biznesowej (CORE 1). Jego głównym celem jest bezpieczny odczyt stanów pinów GPIO, filtracja zakłóceń elektromagnetycznych (programowy debounce) oraz zamiana fizycznych naciśnięć klawiszy na abstrakcyjne, jednorazowe zdarzenia logiczne (impulsy) gotowe do przetworzenia przez maszynę stanów menu głównego sterownika.FunkcjeObsługa trzech fizycznych przycisków sterujących (MODE, PLUS, MINUS) podłączonych w konfiguracji wbudowanego podciągania (INPUT_PULLUP).Wykrywanie zbocza opadającego (momentu fizycznego wciśnięcia przycisku do masy).Niezależna, asynchroniczna filtracja drgań styków (hardware debounce) na poziomie $50\,\text{ms}$ dla każdego kanału.Udostępnianie zdarzeń w architekturze "odczytaj i wyczyść" (Auto-Reset), co eliminuje problem wielokrotnego przetworzenia tego samego kliknięcia w pętli loop().Przechowywanie lokalnych kopii parametrów stanu pracy (WorkMode) oraz mocy zadanej w trybie ręcznym.Logika działania (Wykrywanie Zboczy i Flagi Impulsowe)Fizyczne przyciski mechaniczne podczas wciskania generują mikro-zwarcia i rozwarcia (drgania styków), które mikrokontroler mógłby zinterpretować jako kilkanaście szybkich kliknięć. ControlPanel eliminuje to zjawisko programowo:Weryfikacja Zbocza: Moduł w pętli update() monitoruje zmianę stanu pinów z HIGH na LOW.Odmierzanie Blokady: Po wykryciu zbocza opadającego funkcja sprawdza, czy czas, jaki upłynął od poprzedniego potwierdzonego kliknięcia, jest większy niż stała debounceDelay ($50\,\text{ms}$).Generowanie Impulsu (Auto-Reset): Jeśli warunek czasu jest spełniony, ustawiana jest wewnętrzna flaga (np. m_plusClicked = true).Konsumpcja Zdarzenia: Wywołanie metody publicznej (np. wasPlusPressed()) zwraca stan flagi i natychmiast zeruje ją w tej samej operacji:C++bool ControlPanel::wasPlusPressed() { 
    bool temp = m_plusClicked; 
    m_plusClicked = false; // Czyszczenie flagi (impuls skonsumowany)
    return temp; 
}
Wejścia i WyjściaWejścia (Hardware)Trzy piny wejściowe GPIO zdefiniowane w Config.h:PIN_BUTTON_MODE (33) — zmiana ekranów / trybów pracy.PIN_BUTTON_PLUS (32) — inkrementacja wartości / nawigacja w górę.PIN_BUTTON_MINUS (25) — dekrementacja wartości / nawigacja w dół.Wyjścia (API publiczne)C++void begin();                  // Konfiguracja pinów w tryb INPUT_PULLUP
void update();                 // Wywołanie odczytu i mechanizmu debounce (CORE 1)
WorkMode getMode() const;      // Pobranie aktualnego trybu pracy
void setMode(WorkMode mode);   // Nadpisanie trybu pracy z poziomu logiki nadrzędnej
uint8_t getManualPower() const;// Pobranie nastawy mocy dla trybu MANUAL
void setManualPower(uint8_t power); // Nadpisanie nastawy mocy ręcznej
bool wasModePressed();         // Sprawdzenie kliknięcia MODE (z auto-resetem)
bool wasPlusPressed();         // Sprawdzenie kliknięcia PLUS (z auto-resetem)
bool wasMinusPressed();        // Sprawdzenie kliknięcia MINUS (z auto-resetem)
Współpraca z modułami   [ Przyciski Fizyczne ] ───► (Stan GPIO: LOW/HIGH)
                               │
                               ▼
                        ┌───────────────┐
                        │ ContextPanel  │ (CORE 1 - Debounce i generowanie impulsu)
                        └───────┬───────┘
                                │
                                ▼ (Czyste impulsy logiczne: was...Pressed)
                        [ Logika Główna / main.cpp ]
                                │
                                ├───────────► [ DisplayManager ] (Przełączenie ekranu / edycja wartości)
                                └───────────► [ PhaseController ] (Zmiana mocy w trybie MANUAL)
Moduł NIE odpowiada za:Bezpośrednie rysowanie czegokolwiek na ekranie LCD (zadanie DisplayManager).Zmianę parametrów pracy triaka (przekazuje jedynie dane do pętli głównej, która podejmuje decyzje wykonawcze).Zapisywanie konfiguracji do pamięci Flash po edycji parametrów.Scenariusze Testowe (QA)Test Drgania Styków (Debounce): Symulacja zwarcia pinu PIN_BUTTON_PLUS do masy 10 razy w ciągu $15\,\text{ms}$. Oczekiwany rezultat: Wywołanie wasPlusPressed() w pętli głównej zwróci true dokładnie tylko raz.Test Wielokrotnego Odczytu: Wciśnięcie przycisku MODE (ustawienie flagi). Wywołanie metody wasModePressed() dwa razy pod rząd. Oczekiwany rezultat: Pierwsze wywołanie zwraca true, drugie wywołanie zwraca false (potwierdzenie poprawnego działania mechanizmu konsumpcji impulsu).Status i WersjaStatus: 🟢 Klasa zaimplementowana i gotowa do integracji z maszyną stanów menuWersja: 0.3Autor: Arkadiusz Marek