

# ESPNowManager
Zadanie modułu
Moduł ESPNowManager odpowiada za bezprzewodową, asynchroniczną rejestrację i parsowanie danych telemetrycznych wysyłanych z drugiego mikrokontrolera (nadajnika) zainstalowanego bezpośrednio przy falowniku PV.

Działa w warstwie biznesowej (CORE 1), dostarczając całemu sterownikowi aktualnych informacji o stanie bilansu energetycznego domu oraz parametrach akumulatora 24V. Moduł realizuje zaawansowaną diagnostykę poprawności transmisji radiowej i automatycznie wykrywa awarię linku komunikacyjnego (Timeout).

Funkcje
Obsługa bezprzewodowego protokołu ESP-NOW (bezpośrednia, bezwątkowa komunikacja bez narzutu na zestawianie połączenia Wi-Fi).

Wrapper (mostek C/C++) umożliwiający bezpieczne przekierowanie sprzętowego callbacku przerwania esp_now_recv_cb do wnętrza instancji klasy.

Wyliczanie wskaźników jakości połączenia (QoS): wykrywanie zgubionych ramek na podstawie inkrementacji packetId oraz pomiar rzeczywistego interwału nadawania [ms].

Mechanizm „Watchdoga” połączenia: automatyczny reset danych i przejście w stan awarii po 7 sekundach braku transmisji.

Bezpieczne udostępnianie sparsowanych pól struktury danych za pomocą publicznego interfejsu API.

Architektura Ramki Danych (InverterPacket)
W celu minimalizacji czasu zajętości pasma radiowego oraz uproszczenia parsowania, dane przesyłane są w postaci surowego, upakowanego rekordu pamięci o stałej długości:

C++
struct __attribute__((packed)) InverterPacket {
    uint32_t packetId;       // Monotoniczny licznik pakietów (detekcja dziur w transmisji)
    uint16_t pvPower;        // Bieżąca generacja z paneli fotowoltaicznych [W]
    uint16_t inverterPower;  // Obciążenie wyjściowe falownika / domu [W]
    int16_t  batteryPower;   // Bilans mocy akumulatora [W] (wartość ujemna = ładowanie, dodatnia = rozładowanie)
    uint8_t  soc;            // Poziom naładowania magazynu energii [%]
    float    batteryVoltage; // Napięcie na zaciskach baterii [V]
    float    batteryCurrent; // Prąd płynący z/do akumulatora [A]
};
Logika działania (Diagnostyka i Bezpieczeństwo)
1. Detekcja Zgubionych Pakietów
Przy odebraniu pierwszej prawidłowej ramki moduł synchronizuje się z jej packetId i wylicza wartość oczekiwaną dla następnej próbki: m_expectedPacketId = packet.packetId + 1.

Jeśli w kolejnej iteracji nadajnik prześle ramkę o wyższym identyfikatorze, różnica ta jest natychmiast dopisywana do licznika strat (m_lostPackets). Pozwala to na precyzyjną ocenę zakłóceń elektromagnetycznych w miejscu montażu.

2. Strażnik Łącza (Link Watchdog)
W pętli update() na CORE 1 moduł stale monitoruje czas, jaki upłynął od odebrania ostatniego poprawnego pakietu (millis() - m_lastPacketTime). Jeśli czas ten przekroczy próg bezpieczeństwa 7000 ms:

Flaga połączenia m_connected zmienia stan na false.

Wartości mocy są zerowane w celu ochrony przed podjęciem decyzji wykonawczych na podstawie przestarzałych danych (tzw. "stale data").

Układ przechodzi w tryb poszukiwania synchronizacji ramy (m_firstPacketReceived = false).

Wejścia i Wyjścia
Wejścia (Warstwa Radiowa)
Surowy strumień bajtów odbierany asynchronicznie przez stos radiowy ESP32 o długości równej sizeof(InverterPacket).

Wyjścia (API publiczne dla automatyki EMS)
C++
void begin();                       // Inicjalizacja stosu ESP-NOW i rejestracja callbacku
void update();                      // Kontrola timeoutu (wywoływane w loop na CORE 1)
bool isConnected() const;           // Zwraca status połączenia (true = link aktywny)
uint32_t getPacketCounter() const;  // Całkowita liczba odebranych ramek
uint32_t getLostPackets() const;    // Całkowita liczba zgubionych ramek
uint32_t getLastPeriodMs() const;   // Czas w [ms] pomiędzy dwoma ostatnimi ramkami
uint16_t getInverterPower() const;  // Aktualna moc falownika [W]
int16_t  getBatteryPower() const;   // Aktualny bilans baterii [W] (+/-)
uint8_t  getSOC() const;            // Stan naładowania akumulatora [%]
Współpraca z modułami
  [ Nadajnik przy Falowniku ] ───► (Fale radiowe 2.4GHz / ESP-NOW)
                                         │
                                         ▼
                               ┌──────────────────┐
                               │  ESPNowManager   │ (CORE 1 - Odbiór i analiza QoS)
                               └────────┬─────────┘
                                        │
         ┌──────────────────────────────┼──────────────────────────────┐
         ▼                              ▼                              ▼
  [ AutoController ]               [ Guardian ]               [ DisplayManager ]
 (Wyliczanie nadwyżki EMS)    (Ochrona przed overload)    (Ekran diagnostyczny ESP_DIAG)
Moduł NIE odpowiada za:
Konfigurację sprzętową samej karty sieciowej Wi-Fi i wybór kanału radiowego (wymaga zewnętrznej inicjalizacji np. w main.cpp).

Interpretację logiczną odebranych watów i sterowanie grzałką.

Scenariusze Testowe (QA)
Test Rozmiaru Ramki: Wysłanie uszkodzonego pakietu o długości o 1 bajt mniejszej niż struktura InverterPacket. Oczekiwany rezultat: Metoda handleRx() zignoruje pakiet, dane systemowe nie zostaną nadpisane śmieciowymi wartościami.

Test Odzyskiwania Połączenia (Hot Swap): Wyłączenie zasilania nadajnika na 10 sekund (aktywacja timeoutu, isConnected() == false), a następnie ponowne włączenie. Oczekiwany rezultat: System automatycznie synchronizuje się z nowym ID pakietu, flaga isConnected() wraca na true, a liczniki kontynuują pracę.

Status i Wersja
Status: 🟢 Moduł gotowy / Integracja z callbackiem zgodna z ESP-IDF

Wersja: 0.3

Autor: Arkadiusz Marek