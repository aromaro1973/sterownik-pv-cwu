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
uint8_t currentServiceScreen = 2; // Dla Grupy 2: Ekrany diagnostyki od 2 do 8

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
    logger.setLoggingEnabled(false);
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
    
    // --- ODCZYT PARAMETRÓW Z FLASH (NVS) ---
    guardian.loadSettings(); 
    
    // Synchronizacja odczytanych wartości z modułem wyświetlacza
    displayManager.setMenuMaxPower(guardian.getMaxPower());
    displayManager.setMenuPowerStep(guardian.getPowerStep());
    displayManager.setMenuBatteryDraw(guardian.getMaxBatteryDraw());
    logger.info("Wczytano nastawy NVS - Max: " + String(guardian.getMaxPower()) + "W, Step: " + String(guardian.getPowerStep()) + "W, BatDraw: " + String(guardian.getMaxBatteryDraw()) + "W");

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

    // Odczyt impulsów kliknięć z panelu sterowania
    bool modeClicked  = controlPanel.wasModePressed();      // Krótki klik MODE
    bool modeHeld     = controlPanel.wasModeLongPressed();  // Długi klik MODE (wejście/wyjście z serwisu)
    bool plusClicked  = controlPanel.wasPlusPressed();
    bool minusClicked = controlPanel.wasMinusPressed();

    // Resetowanie timera aktywności w przypadku wykrycia jakiegokolwiek kliknięcia
    bool anyActivity = modeClicked || modeHeld || plusClicked || minusClicked;

    // Pobranie bieżących trybów pracy z panelu sterowania
    WorkMode mode = controlPanel.getMode();
    uint8_t power = controlPanel.getManualPower();

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
        
        if (espNowManager.getPacketCounter() != lastPacketCount)
        {
            lastPacketCount = espNowManager.getPacketCounter();
            lastDataChangeTime = millis(); // Rejestracja odebrania świeżej ramki danych
            
            // SZYBKA REAKCJA: Gdy tylko przychodzi nowy pakiet telemetryczny o mocy inwertera, 
            // natychmiast przekazujemy go do Guardiana, żeby nie czekać na resztę logiki loop().
            guardian.update(espNowManager.getInverterPower(), power);
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

                // Zapewnienie aktualnych wartości na wyświetlaczu wchodząc w menu serwisowe
                displayManager.setMenuMaxPower(guardian.getMaxPower());
                displayManager.setMenuPowerStep(guardian.getPowerStep());
                displayManager.setMenuBatteryDraw(guardian.getMaxBatteryDraw());

                // BEZPIECZEŃSTWO: Całkowite wyłączenie grzałki przy diagnostyce serwisowej
                power = 0;
                mode = WorkMode::OFF;
                controlPanel.setMode(mode);
                controlPanel.setManualPower(power);
                autoController.reset();
                displayManager.forceRefresh();
                break; 
            }

            // --- Obsługa KRÓTKIEGO kliknięcia MODE (Zmiana trybu pracy) ---
            if (modeClicked)
            {
                if (mode == WorkMode::OFF)         mode = WorkMode::AUTO;
                else if (mode == WorkMode::AUTO)   mode = WorkMode::MANUAL;
                else if (mode == WorkMode::MANUAL) mode = WorkMode::OFF;
                
                power = 0; // Bezpieczny reset mocy przy każdej zmianie trybu
                autoController.reset();
                controlPanel.setMode(mode);
                controlPanel.setManualPower(power);
                
                currentSubScreen = 0; 
                displayManager.forceRefresh();
            }

            // --- Obsługa Przycisków PLUS/MINUS w zależności od trybu pracy ---
            if (mode == WorkMode::MANUAL)
            {
                if (plusClicked)
                {
                    if (power < 40) {
                        power += 10;
                    } else if (power >= 40 && power < 47) {
                        power += 1;
                    } else if (power == 47) {
                        power = 100;
                    }
                }
                if (minusClicked)
                {
                    if (power == 100) {
                        power = 47;
                    } else if (power > 40 && power <= 47) {
                        power -= 1;
                    } else if (power >= 10) {
                        power -= 10;
                    }
                }
                controlPanel.setManualPower(power);
            }
            else
            {
                if (plusClicked)
                {
                    currentSubScreen = (currentSubScreen + 1) % 5;
                    displayManager.forceRefresh();
                }
                if (minusClicked)
                {
                    currentSubScreen = (currentSubScreen == 0) ? 4 : currentSubScreen - 1;
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
                    // Przekazujemy do AutoController tylko te parametry,
                    // które są potrzebne do regulacji off-grid: moc falownika,
                    // bilans baterii oraz aktualne limity bezpieczeństwa z Guardian.
                    power = autoController.calculateOffGridPower(
                        espNowManager.getInverterPower(),
                        espNowManager.getBatteryPower(),
                        guardian.getMaxBatteryDraw(),
                        guardian.isBlocked(),
                        guardian.getMaxPower()
                    );
                }
                controlPanel.setManualPower(power);
            }

            displayManager.setSubScreen(currentSubScreen); 
            break;
        }

        // ---------------------------------------------------------------------
        // 3. Grupa 2: Głębokie Menu Serwisowe (Ekrany 2 - 8)
        // ---------------------------------------------------------------------
        case SystemState::GROUP_2_SERVICE:
        {
            power = 0;
            mode = WorkMode::OFF;

            if (anyActivity) {
                serviceTimeoutTimer = millis();
            }

            bool serviceTimeout = (millis() - serviceTimeoutTimer >= 30000);
            bool exitServiceClicked = (currentServiceScreen == 8 && modeClicked);

            if (serviceTimeout || exitServiceClicked || modeHeld)
            {
                logger.info(F("Wyjście z Menu Serwisowego. Przywrócenie sterowania i powrót na Ekran 1.0."));
                displayManager.setScreen(DisplayScreen::MAIN); 

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

            if (modeClicked)
            {
                // --- TRWAŁY ZAPIS DO NVS PRZY KLIKNIĘCIU DALEJ ---
                if (currentServiceScreen == 4) {
                    guardian.setMaxPower(displayManager.getMenuMaxPower());
                    guardian.saveSettings(); // Zapis fizyczny na pamięć Flash
                    logger.info("Zapisano trwale w NVS: Guardian Inv Max = " + String(displayManager.getMenuMaxPower()) + "W");
                }
                else if (currentServiceScreen == 5) {
                    guardian.setMaxBatteryDraw(displayManager.getMenuBatteryDraw());
                    guardian.saveSettings(); // Zapis fizyczny na pamięć Flash
                    logger.info("Zapisano trwale w NVS: Guardian Bat Draw = " + String(displayManager.getMenuBatteryDraw()) + "W");
                }
                else if (currentServiceScreen == 6) {
                    guardian.setPowerStep(displayManager.getMenuPowerStep());
                    guardian.saveSettings(); // Zapis fizyczny na pamięć Flash
                    logger.info("Zapisano trwale w NVS: Guardian Delta P = " + String(displayManager.getMenuPowerStep()) + "W");
                }

                currentServiceScreen++; 
                displayManager.forceRefresh();
            }

            if (currentServiceScreen == 4)
            {
                uint16_t currentMax = displayManager.getMenuMaxPower();
                if (plusClicked && currentMax < 4000)   currentMax += 100;
                if (minusClicked && currentMax >= 100)  currentMax -= 100;
                displayManager.setMenuMaxPower(currentMax);
            }
            else if (currentServiceScreen == 5)
            {
                uint16_t currentBatteryDraw = displayManager.getMenuBatteryDraw();
                if (plusClicked && currentBatteryDraw < 2000)   currentBatteryDraw += 50;
                if (minusClicked && currentBatteryDraw >= 50)   currentBatteryDraw -= 50;
                displayManager.setMenuBatteryDraw(currentBatteryDraw);
            }
            else if (currentServiceScreen == 6)
            {
                uint16_t currentStep = displayManager.getMenuPowerStep();
                if (plusClicked && currentStep < 3000)   currentStep += 50;
                if (minusClicked && currentStep >= 50)   currentStep -= 50;
                displayManager.setMenuPowerStep(currentStep);
            }

            displayManager.setServiceScreen(currentServiceScreen);
            break;
        }
    }

    // =========================================================================
    // Nadrzędna Blokada Bezpieczeństwa (Zrzut do stanu awaryjnego)
    // =========================================================================
    if (currentSystemState == SystemState::GROUP_1_WORK && guardian.isBlocked())
    {
        power = 0; 
        mode = WorkMode::MANUAL;
        
        // Całkowite zablokowanie nastaw logicznych
        controlPanel.setMode(mode);
        controlPanel.setManualPower(power);
        autoController.reset();
        
        displayManager.setMode(WorkMode::OFF); 
    }
    else 
    {
        displayManager.setMode(mode);
    }
  
    // =========================================================================
    // Fizyczny sterownik triaka i synchronizacja wyświetlania
    // =========================================================================
    phaseController.setPower(power);

    displayManager.setPower(power);
    displayManager.setHeaterState(power > 0);
    displayManager.setFrequency(zeroCross.getFrequency());

    //==================================================
    // Logger Diagnostyczny (Co 1 sekundę w tle)
    //==================================================
    if (Utils::elapsed(logTimer, LOG_INTERVAL))
    {
        uint32_t currentZc = Utils::zcCounter;
        uint32_t currentTriggers = Utils::triggerCounter;

        Utils::zcCounter = 0;
        Utils::triggerCounter = 0;

        displayManager.updateDiagnostics(currentZc, currentTriggers, lastEspNowPacketTime);

        if (logger.isLoggingEnabled())
        {
            String logMsg = "State=" + String((int)currentSystemState) +
                            " Mode=" + String((int)mode) +
                            " Power=" + String(power) + "%" +
                            " Freq=" + String(zeroCross.getFrequency(), 1) +
                            " [DIAG: ZC_s=" + String(currentZc) + " TRI_s=" + String(currentTriggers) + "]";
                            
            if (guardian.isBlocked()) {
                logMsg += " [ALARM: GUARDIAN BLOCKED! Powód: " + String((int)guardian.getBlockReason()) + "]";
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
    }

    // Odświeżenie danych telemetrycznych na ekranie
    displayManager.update(espNowManager); 
}