Guardian — Dokumentacja Techniczna Modułu (v0.2)
Zadanie modułu
Moduł Guardian odpowiada za programowe bezpieczeństwo pracy sterownika oraz natychmiastową ochronę falownika przed przeciążeniem.

Pełni rolę elektronicznego bezpiecznika o najwyższym priorytecie. Działa w warstwie biznesowej (CORE 1), skąd w razie wykrycia anomalii ma możliwość natychmiastowego zablokowania generowania impulsów wyzwalających triak w sekcji krytycznej czasowo (CORE 0), niezależnie od aktualnego trybu pracy (AUTO/MANUAL).

Logika działania (Algorytm Dwustopniowy)
Guardian nie analizuje trendów ani nie filtruje danych — działa bezwzględnie na surowych, asynchronicznych ramkach z modułu ESPNowManager. Wykorzystuje dwustopniowy mechanizm weryfikacji, aby uniknąć fałszywych wyzwalaczy, zachowując czas reakcji rzędu milisekund.

Ochrona Statyczna (Moc Maksymalna): Monitorowanie całkowitego obciążenia falownika. Przekroczenie progu bezpiecznego (parametr maxPower) inicjuje procedurę alarmową.

Ochrona Dynamiczna (ΔP - Detekcja "Czajnika"): Obliczenie przyrostu mocy między próbką n a próbką n−1. Jeśli wykryto nagły skok poboru energii przez inne urządzenia domowe przekraczający powerStep, moduł natychmiast reaguje.

Mechanizm Potwierdzenia (Pending Alarm):

Próbka n: Wykrycie przekroczenia → przejście w stan Pending Alarm.

Próbka n+1: Jeśli przekroczenie nadal występuje → natychmiastowa blokada (_isBlocked = true). Jeśli anomalii nie ma → powrót do normy (kasowanie alarmu).

Specyfikacja Powiązań Międzymodułowych
       [ ESPNowManager ] (Dane o sieci i falowniku)
              │
              ▼
         ┌───────────┐
         │ GUARDIAN  │ (CORE 1 - Decyzja o blokadzie)
         └─────┬─────┘
               │
               ├───────────► [ BurstFire / HeaterOutput ] (CORE 0 - Natychmiastowe odcięcie impulsu bramki)
               ├───────────► [ AutoController ]           (CORE 1 - Wstrzymanie algorytmu regulacji)
               ├───────────► [ ControlPanel / Display ]   (CORE 1 - Wizualizacja błędu: Alarm, LCD)
               └───────────► [ Logger ]                   (CORE 1 - Zrzut powodu do Flash)
Za co Guardian NIE odpowiada:
Regulacja prądu i wyliczanie kąta załączenia triaka (zadanie AutoController).

Generowanie impulsów na zboczu opadającym sinusoidy (zadanie timera sprzętowego na CORE 0).

Komunikacja radiowa i parsowanie ramek (zadanie ESPNowManager).

Architektura i API Modułu
Klasa Guardian (Aktualny Interfejs C++)
C++
#ifndef GUARDIAN_H
#define GUARDIAN_H

#include <Arduino.h>

enum class GuardianBlockReason : uint8_t
{
    NONE,
    MAX_POWER,  // Przekroczenie mocy całkowitej falownika
    POWER_STEP  // Wykryto udar dynamiczny (np. czajnik)
};

class Guardian
{
public:
    // Inicjalizacja: podanie fizycznej mocy grzałki (np. 2000W)
    void begin(uint16_t nominalHeaterPower);

    // Wywoływane na CORE 1 po odebraniu nowej ramki z ESPNowManager
    void processSafetyCheck(uint16_t inverterCurrentPower, uint16_t heaterActualPower);

    // Gettery i Settery parametrów (konfigurowalne z ControlPanel/EEPROM)
    void setMaxPower(uint16_t power);
    uint16_t getMaxPower() const;

    void setPowerStep(uint16_t step);
    uint16_t getPowerStep() const;

    // Komunikacja międzyrdzeniowa / Biznesowa
    bool isBlocked() const;
    GuardianBlockReason getBlockReason() const;
    const char* blockReasonToString() const;
    
    // Resetowanie blokady (wymagane potwierdzenie użytkownika z poziomu ControlPanel)
    void resetLockout();

private:
    // Funkcje wewnętrzne (niepubliczne)
    bool checkMaxPower(uint16_t totalPower);
    bool checkPowerStep(uint16_t totalPower);

    uint16_t _heaterNominalPower;
    uint16_t _maxPowerLimit = 3500;  // Domyślnie [W]
    uint16_t _powerStepLimit = 1000; // Domyślnie [W]
    uint16_t _lastInverterPower = 0;

    volatile bool _isBlocked = false; 
    bool _pendingAlarm = false;
    GuardianBlockReason _blockReason = GuardianBlockReason::NONE;
};

#endif
Scenariusze Testowe (QA)
Test Obciążenia Statycznego: Wymuszenie w ramce ESP-NOW poboru na poziomie 3600 W przy limicie 3500 W. Wynik: Blokada po 2 kolejnych ramkach, powód: MAX_POWER.

Test Dynamiki (ΔP): Symulacja skoku mocy falownika z 1200 W na 2500 W w czasie jednej próbki. Wynik: Natychmiastowe ustawienie _isBlocked = true i odcięcie sterowania triakiem, zanim falownik wejdzie w stan awarii overload.

Test Odporności na Szum: Pojedyncza, błędna próbka ze skokiem mocy, po której następuje powrót do normy. Wynik: Aktywacja Pending Alarm, brak twardej blokady, automatyczne skasowanie flagi przy kolejnej próbce.

Który moduł bierzemy na warsztat jako następny? AutoController (logika śledzenia nadwyżek i ochrona akumulatora 24 V) czy sekcję wykonawczą z CORE 0 (BurstFire / HeaterOutput)?