Moduł: Logger (v1.0)
Moduł Logger dostarcza ujednolicony, przejrzysty i zoptymalizowany system logowania zdarzeń diagnostycznych na port szeregowy (UART). Pozwala na kategoryzowanie komunikatów według poziomów ważności oraz dynamiczne włączanie/wyłączanie szczegółowych logów deweloperskich.

💡 Architektura i optymalizacja pamięci
W systemach wbudowanych ciągłe wysyłanie komunikatów tekstowych może szybko doprowadzić do przepełnienia pamięci RAM. Aby temu zapobiec, Logger został zaprojektowany z myślą o maksymalnej wydajności:

Wsparcie dla pamięci Flash (__FlashStringHelper):
Klasa posiada przeciążone metody dla wskaźników pamięci programu (Flash). Dzięki temu programista może używać makra F() (np. logger.info(F("Uruchamianie systemu"));), co sprawia, że tekst jest odczytywany bezpośrednio z pamięci Flash i nie zajmuje ani jednego bajtu cennej pamięci RAM.

Precyzyjne znaczniki czasu (Timestamp):
Każda linia logu jest automatycznie poprzedzana stemplem czasowym w milisekundach od uruchomienia mikrokontrolera (millis()), co umożliwia dokładną analizę sekwencji zdarzeń i czasu reakcji algorytmów (np. pętli EMS).

Filtrowanie poziomów (Verbosity):
Wpisy o statusie DEBUG mogą być dynamicznie ignorowane/wyciszane w czasie pracy urządzenia bez konieczności rekompilacji kodu (za pomocą metody setDebug()).

🚦 Poziomy logowania (Level)
Wpisom diagnostycznym przypisywany jest jeden z czterech priorytetów:

DEBUG – Szczegółowe informacje o stanie zmiennych, krokach algorytmu i odebranych pakietach. Wyłączane w trybie produkcyjnym.

INFO – Standardowe powiadomienia o poprawnym działaniu systemu (np. zmiana trybu pracy, nawiązanie połączenia).

WARNING – Ostrzeżenia o nieprawidłowościach, które nie zatrzymują pracy sterownika (np. chwilowy brak odpowiedzi z sensora, podwyższona temperatura).

ERROR – Krytyczne błędy skutkujące natychmiastowym działaniem ochronnym (np. blokada bezpieczeństwa z modułu Guardian).

🛠️ Opis interfejsu programistycznego (API)
Metody publiczne
void begin(uint32_t baud = 115200)
Opis: Inicjalizuje magistralę szeregową Serial ze zdefiniowaną prędkością transmisji.

void setDebug(bool enable)
Opis: Włącza lub wyłącza filtrowanie wiadomości o poziomie DEBUG.

Metody zapisu logów
Każdy poziom logowania posiada dwa przeciążenia: jedno dla dynamicznych łańcuchów znakowych (const String &msg), drugie dla ciągów statycznych zapisanych w pamięci programu (const __FlashStringHelper *msg / makro F()):

void debug(...) – Loguje komunikat deweloperski (jeśli debugowanie jest włączone).

void info(...) – Loguje standardową informację systemową.

void warning(...) – Loguje ostrzeżenie.

void error(...) – Loguje błąd krytyczny.

📝 Format wyjściowy logów
Każda linijka logu wyprowadzana na port szeregowy zachowuje następującą strukturę:
[CZAS_MS] [POZIOM] TREŚĆ_KOMUNIKATU

Przykład zrzutu z konsoli szeregowej:
Plaintext
[0] [INFO ] Inicjalizacja sterownika...
[500] [INFO ] Połączenie ESP-NOW: OK
[1000] [DEBUG] EMS Loop: obliczone wysterowanie = 40%
[1500] [DEBUG] EMS Loop: obliczone wysterowanie = 40%
[1800] [WARN ] Przekroczono dopuszczalny pobór z akumulatora! (450W)
[2000] [DEBUG] EMS Loop: szybki zrzut mocy (Anty-Czajnik)
[2002] [ERROR] [LIMIT MAX POWER] Grzałka odcięta przez Guardian!