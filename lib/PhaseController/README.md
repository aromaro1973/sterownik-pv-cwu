# Guardian
Zadanie modułu
Moduł Guardian odpowiada za programowe bezpieczeństwo pracy sterownika oraz natychmiastową ochronę falownika przed przeciążeniem i szokiem prądowym.

Pełni rolę elektronicznego bezpiecznika o najwyższym priorytecie w systemie. Działa w warstwie biznesowej (CORE 1), skąd w razie wykrycia anomalii natychmiastowo wymusza blokadę generowania impulsów na bramkę triaka w module wykonawczym PhaseController (CORE 0), niezależnie od aktualnego trybu pracy sterownika (AUTO / MANUAL).

Funkcje
Kontrola maksymalnej dopuszczalnej mocy falownika (ochrona statyczna).

Kontrola nagłego wzrostu mocy zewnętrznej (ochrona dynamiczna przed tzw. "efektem czajnika").

Błyskawiczne wymuszenie blokady pracy grzałki w module PhaseController.

Udostępnianie informacji o stanie i powodzie zabezpieczenia do modułów informacyjnych.

Współpraca z lokalnym menu konfiguracyjnym (ControlPanel).

Logika działania (Algorytm Dwustopniowy)
Guardian nie analizuje długofalowych trendów ani nie filtruje cyfrowo pomiarów — jego zadaniem jest natychmiastowa reakcja. Działa bezwzględnie na surowych, asynchronicznych ramkach danych dostarczanych przez ESPNowManager.

W celu uniknięcia fałszywych alarmów wywołanych pojedynczymi szumami pomiarowymi, moduł stosuje szybką, dwustopniową weryfikację:

Ochrona Statyczna (MAX_POWER): Stałe monitorowanie całkowitego obciążenia falownika. Przekroczenie bezpiecznego progu zdefiniowanego przez użytkownika inicjuje procedurę alarmową.

Ochrona Dynamiczna (POWER_STEP): Obliczenie przyrostu mocy (ΔP) między bieżącą próbką n a poprzednią próbką n−1. Jeśli wykryto nagły skok poboru energii przez inne urządzenia domowe (np. włączenie czajnika elektrycznego), moduł przechodzi do weryfikacji.

Mechanizm Potwierdzenia (Pending Alarm):

Próbka n: Wykrycie przekroczenia kryterium statycznego lub dynamicznego → ustawienie flagi ostrzegawczej (Pending Alarm).

Próbka n+1: * Jeśli w kolejnej ramce przekroczenie nadal występuje → następuje natychmiastowa twarda blokada sterownika (_isBlocked = true).

Jeśli anomalia ustąpiła (szum) → alarm jest anulowany, a sterownik kontynuuje normalną pracę.

Wejścia i Wyjścia
Wejścia (Dane wejściowe)
Moduł przetwarza dane przekazywane po odebraniu każdej ramki z ESPNowManager:

Aktualna całkowita moc falownika [W].

Aktualna moc oddawana przez grzałkę [W] (w celu kalkulacji tła obciążenia).

Parametry progowe zdefiniowane przez użytkownika.

Wyjścia (API publiczne)
C++
bool isBlocked();                   // Zwraca informację, czy grzałka jest zablokowana
GuardianBlockReason getBlockReason(); // Zwraca powód blokady (enum)
const char* blockReasonToString();   // Zwraca tekstowy opis błędu dla LCD/Loggera
uint16_t getMaxPower();             // Zwraca aktualny limit mocy falownika [W]
uint16_t getPowerStep();            // Zwraca aktualny limit skoku mocy [W]
Parametry użytkownika
Parametry konfigurowane lokalnie z poziomu ControlPanel i przechowywane w pamięci nieulotnej (EEPROM/Preferences) ESP32:

Maksymalna moc falownika [W] (np. 3500 W dla falownika 4.2 kW) — próg aktywacji MAX_POWER.

Maksymalny dopuszczalny skok mocy [W] (np. 1000 W) — próg aktywacji POWER_STEP.

Współpraca z modułami
       [ ESPNowManager ] (Odbiór ramek z falownika)
              │
              ▼
         ┌───────────┐
         │ GUARDIAN  │ (CORE 1 - Analiza bezpieczeństwa w pętli loop)
         └─────┬─────┘
               │
               ├───────────► [ PhaseController ]        (CORE 0 - Natychmiastowe odcięcie impulsu bramki)
               ├───────────► [ AutoController ]         (CORE 1 - Wstrzymanie algorytmu regulacji nadwyżki)
               ├───────────► [ ControlPanel / Display ] (CORE 1 - Wizualizacja stanu blokady na LCD)
               └───────────► [ Logger ]                 (CORE 1 - Zapis zdarzenia awaryjnego do Flash)
Moduł NIE odpowiada za:
Detekcję przejścia sieci przez zero (zadanie układu hardware + ZeroCross).

Wyliczanie kąta fazowego i wyzwalanie triaka na zboczu opadającym (zadanie PhaseController).

Śledzenie nadwyżek i płynną regulację procesu EMS (zadanie AutoController).

Obsługę komunikacji bezprzewodowej (zadanie ESPNowManager).

Złota zasada działania
Guardian nie uczestniczy w regulacji procesu i nie steruje płynnie mocą. Podejmuje wyłącznie binarną decyzję o krytycznym znaczeniu: "Grzałka może pracować" albo "Grzałka zostaje bezwzględnie zablokowana".

Scenariusze Testowe (QA)
Test Przeciążenia Statycznego: Wymuszenie w ramce testowej poboru 3700 W przy limicie 3500 W. Oczekiwany rezultat: Blokada po dwóch próbkach z kodem MAX_POWER.

Test Efektu Czajnika: Symulacja nagłego skoku obciążenia sieci z 500 W na 2000 W w czasie jednej próbki. Oczekiwany rezultat: Natychmiastowe ustawienie stanu blokady w celu ochrony falownika przed przeciążeniem zanim ten przejdzie w tryb Overload.

Test Odporności na Szum: Pojedyncza błędna próbka (np. anomalny skok pomiaru), po której kolejna wraca do normy. Oczekiwany rezultat: Aktywacja stanu Pending Alarm i automatyczne samoczynne skasowanie bez wyzwalania twardej blokady.

Status i Wersja
Status: 🟢 Gotowy do implementacji / Architektura zatwierdzona

Wersja: 0.2

Autor: Arkadiusz Marek