#include <Arduino.h>
#include <WiFi.h> 
#include <esp_wifi.h> // Wymagane do bezpośredniej kontroli kanału Wi-Fi dla ESP-NOW

#include <Logger.h>
#include <Config.h>
#include <Utils.h>
#include <ZeroCross.h>
#include <DisplayManager.h>
#include <ControlPanel.h>
#include <Guardian.h>
#include <AutoController.h>
#include <PhaseController.h>
#include <ESPNowManager.h>

//==================================================
// Instancje obiektów globalnych
//==================================================
Logger logger;
ZeroCross zeroCross;
PhaseController phaseController;
DisplayManager displayManager;
ControlPanel controlPanel;
Guardian guardian;
ESPNowManager espNowManager;
AutoController autoController;

//==================================================
// Definicje Ekranów i Stanów Systemu
//==================================================
enum class SystemState {
    SPLASH_SCREEN,      // Ekran startowy (3 sekundy, wymuszony OFF)
    GROUP_1_WORK,       // Grupa 1: Ekrany pracy (1.0, 1.1, 1.2, 1.3, 1.4)
    GROUP_2_SERVICE     // Grupa 2: Głębokie Menu Serwisowe (Ekrany 2 do 7)
};

// Zmienne stanów
SystemState currentSystemState = SystemState::SPLASH_SCREEN;
uint8_t currentSubScreen = 0; // Dla Grupy 1: 0 = Główny, 1 = PV, 2 = Dom, 3 = Bat, 4 = Live Debug
uint8_t currentServiceScreen = 2; // Dla Grupy 2: Ekrany diagnostyki od 2 do 7

//==================================================
// Zmienne czasowe (Tymery i Timeouty)
//==================================================
uint32_t splashScreenTimer = 0;     // Timer dla 3-sekundowego startu
uint32_t group1TimeoutTimer = 0;    // Timeout 30 sekund braku aktywności w Grupie 1
uint32_t serviceTimeoutTimer = 0;   // Timeout 30 sekund braku aktywności w Menu Serwisowym
uint32_t logTimer = 0;
uint32_t lastEspNowPacketTime = 0;  // Failsafe 7 sekund (brak ramek)

//==================================================
// Sekcja Setup
//==================================================
void setup() {
    Serial.begin(115200);
    
    // 1. Inicjalizacja loggera na samym początku
    logger.begin(SERIAL_BAUDRATE);
    logger.info(F("Sterownik Nadwyzki PV (EMS Wersja Lokalna)"));
    logger.info("Wersja: " + String(FW_VERSION));
    
    // 2. Konfiguracja warstwy radiowej ESP-NOW
    WiFi.mode(WIFI_STA); 
    WiFi.disconnect(); 
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    
    Serial.print("Mój adres MAC to: ");
    Serial.println(WiFi.macAddress());
    
    // 3. Inicjalizacja odbiornika ESP-NOW
    espNowManager.begin();
    
    // 4. Inicjalizacja algorytmu Off-Grid
    autoController.begin(2000); 

    // 5. Inicjalizacja układów wykonawczych i peryferiów
    zeroCross.begin();   
    phaseController.begin(PIN_TRIAC); // Inicjalizacja pinu triaka i sprzętowego timera
    
    displayManager.begin();
    controlPanel.begin();
    
    // 6. Inicjalizacja strażnika przeciążeniowego
    guardian.begin(2000); 

    // 7. BEZPIECZNY START
    // Sterownik zawsze startuje w trybie OFF i z mocą 0%.
    controlPanel.setMode(WorkMode::OFF);
    controlPanel.setManualPower(0);
    
    displayManager.setMode(WorkMode::OFF);
    displayManager.setPower(0);
    displayManager.setHeaterState(false);
    
    // Przypisanie początkowego czasu, aby uniknąć fałszywych timeoutów na starcie
    lastEspNowPacketTime = millis();
    splashScreenTimer = millis(); // Start odmierzania 3 sekund ekranu powitalnego
    
    logger.info(F("Inicjalizacja pomyślna. Rozpoczęto Splash Screen."));
}

//==================================================
// Główna pętla programu
//==================================================
void loop()
{
    // Aktualizacja modułu radiowego oraz peryferiów wejściowych
    espNowManager.update();
    zeroCross.update();
    controlPanel.update(); 
    guardian.update();

    // Odczyt impulsów kliknięć z panelu sterowania
    bool modeClicked  = controlPanel.wasModePressed();      // Krótki klik MODE
    bool modeHeld     = controlPanel.wasModeLongPressed();  // Długi klik MODE (wejście/wyjście z serwisu)
    bool plusClicked  = controlPanel.wasPlusPressed();
    bool minusClicked = controlPanel.wasMinusPressed();

    // Resetowanie timera aktywności w przypadku wykrycia jakiegokolwiek kliknięcia
    bool anyActivity = modeClicked || modeHeld || plusClicked || minusClicked;

    // Pobranie bieżących trybów pracy
    WorkMode mode = controlPanel.getMode();
    uint8_t power = controlPanel.getManualPower();

    // --- Diagnostyka zamrożonych danych ---
    // --- Diagnostyka zamrożonych danych ---
static uint32_t lastPacketCount = 0;
static uint32_t lastDataChangeTime = 0;
if (lastDataChangeTime == 0) {
    lastDataChangeTime = millis();
}

// Sprawdzenie odbioru pakietu ESP-NOW i detekcja zamrożenia
if (espNowManager.isConnected())
{
    lastEspNowPacketTime = millis();
    
    // ZMIANA: Sprawdzamy, czy licznik odebranych pakietów się zwiększył.
    // Dzięki temu, nawet jeśli w nocy PV wynosi ciągle 0, system wie, że dane napływają na bieżąco!
    if (espNowManager.getPacketCounter() != lastPacketCount)
    {
        lastPacketCount = espNowManager.getPacketCounter();
        lastDataChangeTime = millis(); // Rejestracja odebrania świeżej ramki danych
    }
}

    // =========================================================================
    // Nadrzędna Maszyna Stanów Sterownika EMS
    // =========================================================================
    switch (currentSystemState)
    {
        // ---------------------------------------------------------------------
        // 1. Ekran Startowy (Splash Screen)
        // ---------------------------------------------------------------------
        case SystemState::SPLASH_SCREEN:
        {
            // Wymuś bezpieczny start - wyłączona grzałka
            power = 0;
            mode = WorkMode::OFF;
            controlPanel.setMode(mode);
            controlPanel.setManualPower(power);

            // Wyświetl Splash Screen
            displayManager.showSplashScreen(); 

            // Po upływie 3 sekund przejdź do normalnej pracy
            if (millis() - splashScreenTimer >= 3000)
            {
                logger.info(F("Splash Screen zakończony. Urządzenie gotowe do pracy w trybie OFF."));
                
                // Przełączenie trybu wyświetlacza na ekran pracy
                displayManager.setScreen(DisplayScreen::MAIN); 
                
                currentSystemState = SystemState::GROUP_1_WORK;
                currentSubScreen = 0; // Zacznij od Ekranu Głównego 1.0
                group1TimeoutTimer = millis(); // Rozpocznij odliczanie 30s dla ekranów pracy
                displayManager.forceRefresh();
            }
            break;
        }

        // ---------------------------------------------------------------------
        // 2. Grupa 1: Ekrany Pracy (Normalne sterowanie)
        // ---------------------------------------------------------------------
        case SystemState::GROUP_1_WORK:
        {
            // Aktualizacja timera aktywności (powrót do 1.0 po 30s bezczynności)
            if (anyActivity) {
                group1TimeoutTimer = millis();
            }

            // Timeout 30 sekund: Jeśli jesteśmy na podekranie (1.1 - 1.4) i nic nie klikamy -> wróć do 1.0
            if (currentSubScreen != 0 && (millis() - group1TimeoutTimer >= 30000))
            {
                currentSubScreen = 0;
                displayManager.forceRefresh();
                logger.info(F("[TIMEOUT] Brak aktywności przez 30s. Powrót do ekranu głównego 1.0."));
            }

            // --- Obsługa DŁUGIEGO kliknięcia MODE (Wejście do Menu Serwisowego) ---
            if (modeHeld)
            {
                logger.info(F("Wejście do Menu Serwisowego (Grupa 2). Bezpieczne wyłączenie grzałki!"));
                currentSystemState = SystemState::GROUP_2_SERVICE;
                currentServiceScreen = 2; // Zacznij od Ekranu 2 (Diagnostyka ZeroCross)
                serviceTimeoutTimer = millis(); // Reset timera bezczynności dla serwisu
                
                // Przełączenie wyświetlacza na tryb serwisowy
                displayManager.setScreen(DisplayScreen::SERVICE); 

                // BEZPIECZEŃSTWO: Całkowite wyłączenie grzałki przy diagnostyce serwisowej
                power = 0;
                mode = WorkMode::OFF;
                controlPanel.setMode(mode);
                controlPanel.setManualPower(power);
                autoController.reset();
                displayManager.forceRefresh();
                break; // Przerwij pętlę dla tej klatki
            }

            // --- Obsługa KRÓTKIEGO kliknięcia MODE (Zmiana trybu pracy) ---
            if (modeClicked)
            {
                // OFF -> AUTO -> MANUAL -> OFF
                if (mode == WorkMode::OFF)         mode = WorkMode::AUTO;
                else if (mode == WorkMode::AUTO)   mode = WorkMode::MANUAL;
                else if (mode == WorkMode::MANUAL) mode = WorkMode::OFF;
                
                power = 0; // Bezpieczny reset mocy przy każdej zmianie trybu
                autoController.reset();
                controlPanel.setMode(mode);
                controlPanel.setManualPower(power);
                
                // Przy zmianie trybu automatycznie wracamy na ekran główny 1.0
                currentSubScreen = 0; 
                displayManager.forceRefresh();
            }

            // --- Obsługa Przycisków PLUS/MINUS w zależności od trybu pracy ---
            if (mode == WorkMode::MANUAL)
            {
                // W trybie MANUAL: Podekrany są zablokowane. Przyciski bezpośrednio zmieniają moc
                // Regulacja o 10% w zakresie 0% - 40%, potem precyzyjnie co 1% do 47%, a potem skok na 100%
                if (plusClicked)
                {
                    if (power < 40) {
                        power += 10;        // Standardowy skok: 0 -> 10 -> 20 -> 30 -> 40
                    } else if (power >= 40 && power < 47) {
                        power += 1;         // Precyzyjna regulacja co 1%: 40 -> 41 -> 42 -> ... -> 47
                    } else if (power == 47) {
                        power = 100;        // Skok bezpośredni z 47% na pełne 100%
                    }
                }
                if (minusClicked)
                {
                    if (power == 100) {
                        power = 47;         // Powrót ze 100% na maksymalne 47%
                    } else if (power > 40 && power <= 47) {
                        power -= 1;         // Precyzyjne schodzenie co 1%: 47 -> 46 -> 45 -> ... -> 40
                    } else if (power >= 10) {
                        power -= 10;        // Powrót do standardowych kroków: 40 -> 30 -> 20 -> 10 -> 0
                    }
                }
                controlPanel.setManualPower(power);
            }
            else
            {
                // W trybach AUTO / OFF: Przyciski PLUS/MINUS nawigują po 5 podekranach (0 do 4)
                if (plusClicked)
                {
                    currentSubScreen = (currentSubScreen + 1) % 5; // Rotacja: 0 -> 1 -> 2 -> 3 -> 4 -> 0
                    displayManager.forceRefresh();
                }
                if (minusClicked)
                {
                    currentSubScreen = (currentSubScreen == 0) ? 4 : currentSubScreen - 1; // Rotacja w tył
                    displayManager.forceRefresh();
                }
            }

            // --- Logika Bezpieczeństwa / Algorytm nadwyżek (Tryb AUTO) ---
            if (mode == WorkMode::AUTO)
            {
                // Failsafe 7 sekund (Brak ramek LUB zamrożony odczyt)
                bool radioTimeout  = (millis() - lastEspNowPacketTime > 7000);
                bool frozenTimeout = (millis() - lastDataChangeTime > 7000);

                if (radioTimeout || frozenTimeout)
                {
                    // Wymuszenie awaryjnego zrzutu do MANUAL 0%
                    mode = WorkMode::MANUAL;
                    power = 0;
                    controlPanel.setMode(mode);
                    controlPanel.setManualPower(power);
                    autoController.reset();

                    if (radioTimeout) {
                        logger.info(F("[FAILSAFE 7s] Brak komunikacji ESP-NOW! Bezpieczny zrzut do MANUAL 0%."));
                    } else {
                        logger.info(F("[FAILSAFE 7s] Zamrożenie danych telemetrycznych! Bezpieczny zrzut do MANUAL 0%."));
                    }
                }
                else
                {
                    // Normalne wyliczanie mocy na podstawie nadwyżki PV
                    power = autoController.calculateOffGridPower(
                        espNowManager.getPVPower(),
                        espNowManager.getInverterPower(),
                        espNowManager.getBatteryPower(), 
                        MAX_BATTERY_DRAW_W, // Wartość 400W z config.h
                        guardian.isBlocked()
                    );
                }
                controlPanel.setManualPower(power);
            }

            // Aktualizacja parametrów na wyświetlaczu w zależności od aktywnego podekranu
            displayManager.setSubScreen(currentSubScreen); 
            break;
        }

        // ---------------------------------------------------------------------
        // 3. Grupa 2: Głębokie Menu Serwisowe (Ekrany 2 - 7)
        // ---------------------------------------------------------------------
        case SystemState::GROUP_2_SERVICE:
        {
            // Grzałka w tym trybie jest całkowicie wyłączona (power = 0, stan = OFF)
            power = 0;
            mode = WorkMode::OFF;

            // Timer aktywności dla menu serwisowego (powrót do normalnej pracy po 30s)
            if (anyActivity) {
                serviceTimeoutTimer = millis();
            }

            // Warunek wyjścia z menu serwisowego: brak aktywności przez 30 sekund LUB kliknięcie MODE na Ekranie 7
            bool serviceTimeout = (millis() - serviceTimeoutTimer >= 30000);
            bool exitServiceClicked = (currentServiceScreen == 7 && modeClicked);

            if (serviceTimeout || exitServiceClicked || modeHeld)
            {
                logger.info(F("Wyjście z Menu Serwisowego. Przywrócenie sterowania i powrót na Ekran 1.0."));
                
                // Przełączenie trybu wyświetlacza z powrotem na ekrany główne
                displayManager.setScreen(DisplayScreen::MAIN); 

                // Przywrócenie ustawień początkowych przy wyjściu z serwisu (zrzut do domyślnego trybu OFF)
                currentSystemState = SystemState::GROUP_1_WORK;
                currentSubScreen = 0; 
                
                mode = WorkMode::OFF;
                power = 0;
                controlPanel.setMode(mode);
                controlPanel.setManualPower(power);
                autoController.reset();
                
                displayManager.forceRefresh();
                break;
            }

            // --- Nawigacja w Menu Serwisowym za pomocą krótkiego kliknięcia MODE ---
            if (modeClicked)
            {
                // Przejście do kolejnego ekranu (zapis następuje automatycznie w momencie przejścia)
                if (currentServiceScreen == 4) {
                    // Ekran 4 (Ustawienia Guardian - Max moc): Zapisz do modułu Guardian
                    guardian.setMaxPower(displayManager.getMenuMaxPower());
                    logger.info("Zapisano automatycznie: Guardian Inv Max = " + String(displayManager.getMenuMaxPower()) + "W");
                }
                else if (currentServiceScreen == 5) {
                    // Ekran 5 (Ustawienia Guardian - Delta P): Zapisz do modułu Guardian
                    guardian.setPowerStep(displayManager.getMenuPowerStep());
                    logger.info("Zapisano automatycznie: Guardian Delta P = " + String(displayManager.getMenuPowerStep()) + "W");
                }

                currentServiceScreen++; // Przejście na następny ekran (2 -> 3 -> 4 -> 5 -> 6 -> 7)
                displayManager.forceRefresh();
            }

            // --- Regulacja parametrów na ekranach konfiguracyjnych (PLUS/MINUS) ---
            if (currentServiceScreen == 4)
            {
                // Regulacja maksymalnej mocy falownika co 100W (zakres np. 100W - 4000W)
                uint16_t currentMax = displayManager.getMenuMaxPower();
                if (plusClicked && currentMax < 4000)   currentMax += 100;
                if (minusClicked && currentMax >= 100)  currentMax -= 100;
                displayManager.setMenuMaxPower(currentMax);
            }
            else if (currentServiceScreen == 5)
            {
                // Regulacja dP (Anty-czajnik / Delta P) co 50W (zakres np. 50W - 3000W)
                uint16_t currentStep = displayManager.getMenuPowerStep();
                if (plusClicked && currentStep < 3000)   currentStep += 50;
                if (minusClicked && currentStep >= 50) currentStep -= 50;
                displayManager.setMenuPowerStep(currentStep);
            }

            // Przekazanie do managera wyświetlacza informacji, który ekran diagnostyczny renderować
            displayManager.setServiceScreen(currentServiceScreen);
            break;
        }
    }

    // =========================================================================
    // Fizyczny sterownik triaka i synchronizacja wyświetlania
    // =========================================================================
    
    // Przekazanie aktualnej mocy do PhaseControllera w celu wyznaczenia kąta/opóźnienia.
    phaseController.setPower(power);

    // Blokada bezpieczeństwa Guardiana (Ma charakter nadrzędny w trybie pracy)
    if (currentSystemState == SystemState::GROUP_1_WORK && guardian.isBlocked())
    {
        power = 0; 
        displayManager.setMode(WorkMode::OFF); 
    }
    else 
    {
        displayManager.setMode(mode);
    }
  
    // Wysterowanie fizycznych struktur wyświetlacza
    displayManager.setPower(power);
    displayManager.setHeaterState(power > 0);
    displayManager.setFrequency(zeroCross.getFrequency());

    //==================================================
    // Logger Diagnostyczny (Co 1 sekundę w tle)
    //==================================================
    if (Utils::elapsed(logTimer, LOG_INTERVAL))
    {
        // 1. Kopiujemy aktualne wartości z sekcji przerw
        uint32_t currentZc = Utils::zcCounter;
        uint32_t currentTriggers = Utils::triggerCounter;

        // 2. Natychmiast zerujemy liczniki w Utils, aby Core 0 i Core 1 zbierały dane na kolejną sekundę
        Utils::zcCounter = 0;
        Utils::triggerCounter = 0;

        // 3. Wysyłamy świeże dane sekundowe na wyświetlacz (przekazując czas ostatniej ramki radiowej)
        displayManager.updateDiagnostics(currentZc, currentTriggers, lastEspNowPacketTime);

        // 4. Tworzenie pełnego wpisu logowania
        String logMsg = "State=" + String((int)currentSystemState) +
                        " Mode=" + String((int)mode) +
                        " Power=" + String(power) + "%" +
                        " Freq=" + String(zeroCross.getFrequency(), 1) +
                        " [DIAG: ZC_s=" + String(currentZc) + " TRI_s=" + String(currentTriggers) + "]";
                        
        if (guardian.isBlocked()) {
            logMsg += " [ALARM: GUARDIAN BLOCKED!]";
        } else {
            if (espNowManager.isConnected()) {
                logMsg += " [Radio: OK]";
            } else {
                logMsg += " [Radio: DISCONNECTED]";
            }
        }
        logMsg += " PV=" + String(espNowManager.getPVPower()) + "W";
        logger.info(logMsg);
    }

    // Przesłanie wszystkich niezbędnych danych telemetrycznych do odświeżenia ekranu OLED/LCD
    displayManager.update(espNowManager); 
}