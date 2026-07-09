# WiFiManager

## Zadanie modułu

Moduł `WiFiManager` odpowiada za nawiązanie oraz utrzymanie połączenia ESP32 z siecią WiFi.

Jego zadaniem jest automatyczne łączenie z siecią, monitorowanie stanu połączenia oraz ponawianie próby połączenia w przypadku jego utraty.

---

## Funkcje

- inicjalizacja połączenia WiFi
- monitorowanie stanu połączenia
- automatyczne ponowne łączenie z siecią
- odczyt adresu IP
- odczyt siły sygnału RSSI

---

## Wejścia

```cpp
begin(ssid, password);
update();
```

---

## Wyjścia

Moduł udostępnia:

```cpp
isConnected();
getIP();
getRSSI();
```

---

## Współpraca z modułami

```
Config
   │
   ▼
WiFiManager
   │
   ├────────► Logger
   │
   ├────────► DisplayManager
   │
   └────────► Home Assistant
```

---


## Moduł nie odpowiada za

- komunikację MQTT,
- Home Assistant,
- ESP-NOW,
- sterowanie grzałką,
- wyświetlacz LCD,
- automatykę sterownika.

---

## Aktualna logika pracy

1. Uruchomienie trybu WiFi Station.
2. Próba połączenia z zapisaną siecią.
3. Ciągłe monitorowanie połączenia.
4. W przypadku utraty połączenia ponowienie próby co 5 sekund.
5. Udostępnienie informacji:
   - stan połączenia,
   - adres IP,
   - siła sygnału.

---

## Założenia projektowe

Moduł WiFiManager odpowiada wyłącznie za obsługę połączenia WiFi.

Do jego zadań należy:

- inicjalizacja połączenia z siecią WiFi,
- monitorowanie stanu połączenia,
- automatyczne ponawianie połączenia,
- udostępnianie informacji o stanie połączenia pozostałym modułom.

Moduł nie zawiera żadnej logiki związanej z:

- Home Assistant,
- MQTT,
- ESP-NOW,
- sterowaniem grzałką,
- automatyką sterownika.

Sterownik musi pracować poprawnie również w przypadku całkowitego braku połączenia z siecią WiFi.

Brak połączenia z siecią nie może wpływać na działanie modułów:

- ZeroCross,
- BurstFire,
- HeaterOutput,
- DisplayManager,
- ControlPanel.

Jedyną konsekwencją utraty połączenia jest brak komunikacji sieciowej.

Po ponownym pojawieniu się sieci WiFi moduł automatycznie podejmuje próbę ponownego połączenia bez konieczności restartu sterownika.
## Testy

Moduł został sprawdzony podczas testów:

- poprawne połączenie z siecią WiFi,
- poprawny odczyt adresu IP,
- poprawny odczyt RSSI,
- poprawna współpraca z Logger,
- poprawna współpraca z DisplayManager.

---

## Status

🟡 Wersja funkcjonalna

Moduł realizuje wszystkie wymagane funkcje dla wersji testowej V0.2.x.

Planowane jest rozszerzenie o konfigurację WiFi z poziomu Access Point oraz zapis konfiguracji w pamięci nieulotnej.

---

## Autor

Arkadiusz Marek

Projekt rozwijany przy współpracy z ChatGPT.

---

## Wersja modułu

**1.0**

---

## Planowane rozszerzenia API

### getSSID()

```cpp
String getSSID() const;
```

Cel:

Udostępnienie nazwy aktualnie połączonej sieci WiFi.

Przykładowe zastosowanie:

- Logger,
- DisplayManager,
- diagnostyka połączenia.

---

### hasConnectionChanged()

```cpp
bool hasConnectionChanged();
```

Cel:

Informowanie pozostałych modułów o zmianie stanu połączenia.

Funkcja zwraca `true` tylko jeden raz po:

- nawiązaniu połączenia,
- utracie połączenia.

Dzięki temu przyszłe moduły (Home Assistant, MQTT) będą mogły automatycznie ponownie inicjalizować połączenie.

---

### setHostname()

```cpp
void setHostname(const char *hostname);
```

Cel:

Ustawienie nazwy urządzenia w sieci lokalnej.

Przykład:

```
SterownikPV
```

Ułatwia identyfikację urządzenia w routerze oraz podczas integracji z Home Assistant.

---

### reconnectNow()

```cpp
void reconnectNow();
```

Cel:

Natychmiastowe wymuszenie ponownego połączenia z siecią WiFi.

Funkcja może zostać wykorzystana po zmianie konfiguracji sieci lub podczas konfiguracji sterownika.

---

### Access Point (planowane)

Cel:

Automatyczne uruchomienie punktu dostępowego przy pierwszym uruchomieniu lub po skasowaniu konfiguracji WiFi.

Planowana nazwa sieci:

```
SterownikPV
```

Po połączeniu użytkownik będzie mógł wpisać:

- nazwę sieci (SSID),
- hasło WiFi.

Po zapisaniu sterownik automatycznie uruchomi się ponownie i połączy z nową siecią.

---

### Zapisywanie konfiguracji

Cel:

Przechowywanie konfiguracji WiFi w pamięci nieulotnej ESP32 (Preferences / NVS).

Po zaniku zasilania sterownik automatycznie połączy się z ostatnio skonfigurowaną siecią.
