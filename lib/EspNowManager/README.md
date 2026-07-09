# ESPNowManager

## Zadanie modułu

Moduł `ESPNowManager` odpowiada za bezprzewodową komunikację pomiędzy sterownikiem grzałki (Odbiornik) a drugim modułem ESP32 pracującym bezpośrednio przy falowniku Anenji (Nadajnik).

Jego zadaniem jest błyskawiczne odbieranie zunifikowanych danych pomiarowych oraz udostępnianie ich pozostałym układom sterownika.

ESPNowManager nie podejmuje żadnych decyzji dotyczących regulacji mocy ani bezpieczeństwa pracy. Jest wyłącznie warstwą komunikacyjną.

---

## Funkcje

- Inicjalizacja natywnego protokołu ESP-NOW na ESP32.
- Odbiór danych z drugiego ESP32 w postaci jednej, zwięzłej struktury (czas odczytu rejestrów falownika wynosi ~42 ms).
- Kontrola stanu połączenia za pomocą programowego Watchdoga (brak ramki przez 7 sekund = rozłączenie).
- Udostępnianie aktualnych danych procesowych pozostałym modułom.

---

## Wejścia

Moduł odbiera dane z drugiego ESP32 poprzez ESP-NOW za pomocą spakowanej struktury binarnej `InverterPacket`.

Przesyłane informacje obejmują:
- Moc paneli PV (`pvPower`),
- Moc wyjściową falownika AC (`inverterPower`),
- [cite_start]Skonsolidowaną moc baterii (`batteryPower`) – **Ważne:** wartość dodatnia ($+$) oznacza rozładowywanie akumulatora, wartość ujemna ($-$) oznacza ładowanie.
- [cite_start]Stan naładowania SOC[cite: 9],
- [cite_start]Napięcie oraz prąd akumulatora[cite: 5].

---

## Wyjścia (Aktualne API)

Moduł udostępnia publicznie następujące metody:

```cpp
bool isConnected() const;
uint32_t getPacketCounter() const;
uint32_t getLastPacketTime() const;

uint16_t getPVPower() const;
uint16_t getInverterPower() const;
int16_t  getBatteryPower() const; // Znak +/- : (+) rozładowanie, (-) ładowanie
uint16_t getHousePower() const;

uint8_t  getSOC() const;
float    getBatteryVoltage() const;
float    getBatteryCurrent() const;
Współpraca z modułami
ESP przy falowniku (Nadajnik RS232)
         │
         ▼  [ ESP-NOW Transmisja Radiowa ]
   ESPNowManager (Odbiornik grzałki)
         │
         ├────► Guardian (Kontrola limitów)
         ├────► AutoController (Wyliczanie algorytmu Off-Grid)
         ├────► DisplayManager (Wizualizacja na LCD)
         ├────► Logger (Zrzut diagnostyczny)
         └────► HomeAssistant (W przyszłości)
Moduł nie odpowiada za
Sterowanie fizyczne grzałką i triakiem,

Wyliczanie algorytmu regulacji mocy (od tego jest AutoController),

Blokady bezpieczeństwa (od tego jest Guardian),

Zarządzanie połączeniem z domowym routerem (od tego jest WiFiManager).

Założenia projektowe
ESPNowManager jest jedynym modułem odpowiedzialnym za odbiór danych z drugiego ESP32. Pozostałe moduły nie komunikują się bezpośrednio przez radio. Każdy komponent systemu korzysta wyłącznie z danych udostępnionych przez czyste API tej klasy. Dzięki temu cała warstwa komunikacji bezprzewodowej znajduje się w jednym miejscu projektu.

Aktualna logika pracy
Inicjalizacja stosu ESP-NOW (współdzieli kanał radiowy z WiFiManager).

Rejestracja systemowej funkcji callback dla zdarzenia odebrania danych (onDataRecv).

Odbiór kolejnych ramek danych i bezpieczne przepisywanie ich z bufora do zmiennych wewnętrznych klasy.

Cykliczny nasłuch w update() – jeśli czas od odebrania ostatniej ramki przekroczy 7000 ms, następuje automatyczne ustawienie flagi connected = false oraz bezpieczne zerowanie bilansu baterii w celu ochrony systemu przed pracą na "zamrożonych" danych.

Filozofia działania
ESPNowManager jest czystym "dostawcą prawdy" dla reszty programu. Moduł nie interpretuje odebranych wartości, nie analizuje ich pod kątem poprawności fizycznej i nie podejmuje żadnych decyzji wykonawczych. Wyłącznie odbiera strukturę bajtów i wystawia ją przez gettery.

Testy
Wykonane i planowane testy:

Poprawność inicjalizacji interfejsu radiowego,

Zgodność rozmiaru struktury binarnej InverterPacket po obu stronach,

Reakcja programu na nagłe odłączenie zasilania nadajnika (test Watchdoga 7s),

Integracja z trybem WorkMode::AUTO w maszynie stanów sterownika grzałki.

Status
🟢 Moduł wdrożony i zintegrowany z AutoController

Wersja modułu
0.2

Autor
Arkadiusz Marek
Projekt rozwijany przy współpracy z ChatGPT / Gemini.


---

Plik dokumentacji odzwierciedla teraz rzeczywisty stan kodu w 100%. Wszystkie trzy pliki w