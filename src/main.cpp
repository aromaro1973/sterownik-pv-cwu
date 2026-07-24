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
    SPLASH_SCREEN,      // Ekran startowy (3 sekundy, AUTO z moca 0%)
    MAIN_SCREEN,
    INFO_SCREENS,
    CONFIG_SCREENS
};

// Zmienne stanów
SystemState currentSystemState = SystemState::SPLASH_SCREEN;
uint8_t currentSubScreen = 1; // INFO: 1 = Radio, 2 = ZeroCross, 3 = Phase, 4 = Auto, 5 = Heater, 6 = Inv, 7 = Bat, 8 = PV
uint8_t currentServiceScreen = 1; // CONFIG: 1 = MaxPower, 2 = BatteryDraw, 3 = PV Hold, 4 = HeaterPower
bool manualSetupMode = false;
bool commFailsafeActive = false;   // Flaga aktywnego failsafe telemetrii w AUTO

//==================================================
// Zmienne czasowe (Tymery i Timeouty)
//==================================================
uint32_t splashScreenTimer = 0;     // Timer dla 3-sekundowego startu
uint32_t group1TimeoutTimer = 0;    // Rezerwa pod timeout UI (aktualnie niewykorzystywane)
uint32_t serviceTimeoutTimer = 0;   // Rezerwa pod timeout UI (aktualnie niewykorzystywane)
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
    
    // 6. Inicjalizacja magazynu ustawień ochronnych
    guardian.begin(2000); 
    
    // --- ODCZYT PARAMETRÓW Z FLASH (NVS) ---
    guardian.loadSettings(); 
    
    // Synchronizacja odczytanych wartości z modułem wyświetlacza
    displayManager.setMenuMaxPower(guardian.getMaxPower());
    displayManager.setMenuPowerStep(guardian.getPowerStep());
    displayManager.setMenuBatteryDraw(guardian.getMaxBatteryDraw());
    displayManager.setMenuPvHoldDelay(guardian.getPvHoldDelay());
    displayManager.setMenuHeaterPower(guardian.getNominalHeaterPower());
    autoController.setHeaterPower(guardian.getNominalHeaterPower());
    logger.info("Wczytano nastawy NVS - Max: " + String(guardian.getMaxPower()) + "W, Step: " + String(guardian.getPowerStep()) + "W, BatDraw: " + String(guardian.getMaxBatteryDraw()) + "W");

    // 7. START PO URUCHOMIENIU
    // Tryb AUTO jest aktywny od startu, ale moc startuje od 0%.
    // Dzięki temu po powrocie telemetrii sterownik sam wznawia regulację.
    controlPanel.setMode(WorkMode::AUTO);
    controlPanel.setManualPower(0);
    
    displayManager.setMode(WorkMode::AUTO);
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
    bool packetChanged = false;
    if (espNowManager.isConnected())
    {
        lastEspNowPacketTime = millis();
        
        if (espNowManager.getPacketCounter() != lastPacketCount)
        {
            lastPacketCount = espNowManager.getPacketCounter();
            lastDataChangeTime = millis(); // Rejestracja odebrania świeżej ramki danych
            packetChanged = true;
        }
    }

    // Globalny failsafe komunikacji: niezależnie od bieżącego ekranu UI.
    // W trybie AUTO utrzymujemy tryb, ale zrzucamy moc do 0%,
    // aby po powrocie danych automatycznie wznowić pracę.
    bool radioTimeout  = (millis() - lastEspNowPacketTime > 7000);
    bool frozenTimeout = (millis() - lastDataChangeTime > 7000);
    if (mode == WorkMode::AUTO && (radioTimeout || frozenTimeout))
    {
        manualSetupMode = false;
        power = 0;
        controlPanel.setManualPower(power);
        if (!commFailsafeActive)
        {
            autoController.reset();
            if (radioTimeout) {
                logger.info(F("[FAILSAFE 7s] Brak komunikacji ESP-NOW! AUTO pozostaje aktywne, moc ustawiona na 0%."));
            } else {
                logger.info(F("[FAILSAFE 7s] Zamrożenie danych telemetrycznych! AUTO pozostaje aktywne, moc ustawiona na 0%."));
            }
        }
        commFailsafeActive = true;

        if (currentSystemState == SystemState::MAIN_SCREEN)
        {
            displayManager.forceRefresh();
        }
    }
    else if (commFailsafeActive && mode == WorkMode::AUTO && !radioTimeout && !frozenTimeout)
    {
        commFailsafeActive = false;
        autoController.reset();
        logger.info(F("[FAILSAFE] Telemetria wróciła. Wznowienie regulacji AUTO."));
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
            mode = WorkMode::AUTO;
            controlPanel.setMode(mode);
            controlPanel.setManualPower(power);

            // Wyświetl Splash Screen
            displayManager.showSplashScreen(); 

            // Po upływie 3 sekund przejdź do normalnej pracy
            if (millis() - splashScreenTimer >= 3000)
            {
                logger.info(F("Splash Screen zakończony. Urządzenie gotowe do pracy w trybie AUTO (start od 0%)."));
                
                // Przełączenie trybu wyświetlacza na ekran pracy
                displayManager.setScreen(DisplayScreen::MAIN); 
                
                currentSystemState = SystemState::MAIN_SCREEN;
                displayManager.setSubScreen(1);
                displayManager.forceRefresh();
            }
            break;
        }

        // ---------------------------------------------------------------------
        // 2. Ekran główny
        // ---------------------------------------------------------------------
        case SystemState::MAIN_SCREEN:
        {
            if (modeHeld)
            {
                manualSetupMode = false;
                mode = WorkMode::OFF;
                power = 0;
                controlPanel.setMode(mode);
                controlPanel.setManualPower(power);
                autoController.reset();
                displayManager.setScreen(DisplayScreen::MAIN);
                displayManager.forceRefresh();
            }

            // --- Obsługa MODE (Zmiana trybu pracy) ---
            if (modeClicked && !modeHeld)
            {
                if (mode == WorkMode::OFF)
                {
                    mode = WorkMode::AUTO;
                    power = 0;
                    controlPanel.setMode(mode);
                    controlPanel.setManualPower(power);
                    autoController.reset();
                }
                else if (mode == WorkMode::AUTO)
                {
                    mode = WorkMode::MANUAL;
                    manualSetupMode = true;
                    power = controlPanel.getManualPower();
                    if (power == 0) { power = 40; }
                    controlPanel.setMode(WorkMode::MANUAL);
                    controlPanel.setManualPower(power);
                    autoController.reset();
                }
                else if (mode == WorkMode::MANUAL && manualSetupMode)
                {
                    manualSetupMode = false;
                    controlPanel.setMode(WorkMode::MANUAL);
                    controlPanel.setManualPower(power);
                }
                else if (mode == WorkMode::MANUAL && !manualSetupMode)
                {
                    mode = WorkMode::OFF;
                    power = 0;
                    manualSetupMode = false;
                    controlPanel.setMode(mode);
                    controlPanel.setManualPower(power);
                    autoController.reset();
                }

                if (mode == WorkMode::MANUAL && manualSetupMode)
                {
                    controlPanel.setMode(WorkMode::MANUAL);
                    controlPanel.setManualPower(power);
                }

                displayManager.forceRefresh();
            }

            // --- Wejście do ekranów informacyjnych i konfiguracyjnych ---
            if (plusClicked && !manualSetupMode)
            {
                currentSystemState = SystemState::INFO_SCREENS;
                currentSubScreen = 1;
                displayManager.setScreen(DisplayScreen::INFO);
                displayManager.setSubScreen(currentSubScreen);
                displayManager.forceRefresh();
            }
            else if (minusClicked && !manualSetupMode)
            {
                currentSystemState = SystemState::CONFIG_SCREENS;
                currentServiceScreen = 1;
                displayManager.setScreen(DisplayScreen::CONFIG);
                displayManager.setServiceScreen(currentServiceScreen);
                displayManager.setMenuMaxPower(guardian.getMaxPower());
                displayManager.setMenuBatteryDraw(guardian.getMaxBatteryDraw());
                displayManager.setMenuPvHoldDelay(guardian.getPvHoldDelay());
                displayManager.setMenuHeaterPower(guardian.getNominalHeaterPower());
                displayManager.setMenuPowerStep(guardian.getPowerStep());
                displayManager.forceRefresh();
            }

            // Tryb MANUAL: najpierw ustawienie mocy, potem praca sztywna.
            if (mode == WorkMode::MANUAL)
            {
                if (manualSetupMode)
                {
                    if (plusClicked)
                    {
                        if (power < 95) {
                            power += 5;
                        } else {
                            power = 100;
                        }
                    }
                    if (minusClicked)
                    {
                        if (power > 5) {
                            power -= 5;
                        } else {
                            power = 0;
                        }
                    }
                    controlPanel.setManualPower(power);
                }
                else
                {
                    // Tryb manualny po zatwierdzeniu pozostaje sztywny.
                    controlPanel.setManualPower(power);
                }
            }

            if (mode == WorkMode::AUTO)
            {
                if (packetChanged)
                {
                    // Regulacja odbywa się tylko po świeżej telemetrii.
                    // AutoController pracuje wewnętrznie w watach,
                    // a do wykonania przekazuje żądany procent mocy.
                    power = autoController.calculateOffGridPower(
                        espNowManager.getInverterPower(),
                        espNowManager.getBatteryPower(),
                        guardian.getMaxBatteryDraw(),
                        guardian.getMaxPower(),
                        guardian.getPowerStep(),
                        guardian.getPvHoldDelay()
                    );
                }
                controlPanel.setManualPower(power);
            }

            displayManager.setSubScreen(currentSubScreen); 
            break;
        }

        // ---------------------------------------------------------------------
        // 3. Ekrany informacyjne
        // ---------------------------------------------------------------------
        case SystemState::INFO_SCREENS:
        {
            power = 0;
            controlPanel.setManualPower(power);

            if (modeClicked)
            {
                if (currentSubScreen < 8) {
                    currentSubScreen++;
                } else {
                    currentSystemState = SystemState::MAIN_SCREEN;
                    displayManager.setScreen(DisplayScreen::MAIN);
                }
                displayManager.setSubScreen(currentSubScreen);
                displayManager.forceRefresh();
            }

            displayManager.setSubScreen(currentSubScreen);
            break;
        }

        // ---------------------------------------------------------------------
        // 4. Ekrany konfiguracyjne
        // ---------------------------------------------------------------------
        case SystemState::CONFIG_SCREENS:
        {
            power = 0;
            controlPanel.setManualPower(power);

            if (currentServiceScreen == 1)
            {
                uint16_t currentMax = displayManager.getMenuMaxPower();
                if (plusClicked && currentMax < 4000)   currentMax += 100;
                if (minusClicked && currentMax >= 100)  currentMax -= 100;
                displayManager.setMenuMaxPower(currentMax);
            }
            else if (currentServiceScreen == 2)
            {
                uint16_t currentBatteryDraw = displayManager.getMenuBatteryDraw();
                if (plusClicked && currentBatteryDraw < 2000)   currentBatteryDraw += 50;
                if (minusClicked && currentBatteryDraw >= 50)   currentBatteryDraw -= 50;
                displayManager.setMenuBatteryDraw(currentBatteryDraw);
            }
            else if (currentServiceScreen == 3)
            {
                uint16_t currentDelay = displayManager.getMenuPvHoldDelay();
                if (plusClicked && currentDelay < 5000)   currentDelay += 100;
                if (minusClicked && currentDelay >= 100)  currentDelay -= 100;
                displayManager.setMenuPvHoldDelay(currentDelay);
            }
            else if (currentServiceScreen == 4)
            {
                uint16_t currentHeaterPower = displayManager.getMenuHeaterPower();
                if (plusClicked && currentHeaterPower < 4000)   currentHeaterPower += 50;
                if (minusClicked && currentHeaterPower >= 50)   currentHeaterPower -= 50;
                displayManager.setMenuHeaterPower(currentHeaterPower);
            }

            if (modeClicked)
            {
                if (currentServiceScreen < 4) {
                    currentServiceScreen++;
                    displayManager.setServiceScreen(currentServiceScreen);
                } else {
                    guardian.setMaxPower(displayManager.getMenuMaxPower());
                    guardian.setMaxBatteryDraw(displayManager.getMenuBatteryDraw());
                    guardian.setPvHoldDelay(displayManager.getMenuPvHoldDelay());
                    guardian.setNominalHeaterPower(displayManager.getMenuHeaterPower());
                    guardian.setPowerStep(displayManager.getMenuPowerStep());
                    guardian.saveSettings();
                    autoController.setHeaterPower(displayManager.getMenuHeaterPower());
                    logger.info(F("Zapisano ustawienia konfiguracyjne do NVS"));

                    currentSystemState = SystemState::MAIN_SCREEN;
                    displayManager.setScreen(DisplayScreen::MAIN);
                    manualSetupMode = false;
                    autoController.reset();
                }
                displayManager.forceRefresh();
            }

            displayManager.setServiceScreen(currentServiceScreen);
            break;
        }
    }

    displayManager.setMode(mode);
  
    // =========================================================================
    // Fizyczny sterownik triaka i synchronizacja wyświetlania
    // =========================================================================
    uint8_t phaseCommand = power;
    if (mode == WorkMode::MANUAL && manualSetupMode)
    {
        phaseCommand = 0;
    }

    phaseController.setPower(phaseCommand);

    uint8_t appliedPower = phaseController.getAppliedPower();

    displayManager.setPower(power);
    displayManager.setHeaterState(appliedPower > 0);
    displayManager.setPowerAverage(appliedPower);
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
                            " Req=" + String(power) + "%" +
                            " Act=" + String(appliedPower) + "%" +
                            " Freq=" + String(zeroCross.getFrequency(), 1) +
                            " [DIAG: ZC_s=" + String(currentZc) + " TRI_s=" + String(currentTriggers) + "]";

            if (espNowManager.isConnected()) {
                logMsg += " [Radio: OK]";
            } else {
                logMsg += " [Radio: DISCONNECTED]";
            }
            logMsg += " PV=" + String(espNowManager.getPVPower()) + "W";
            logger.info(logMsg);
        }
    }

    // Odświeżenie danych telemetrycznych na ekranie
    displayManager.update(espNowManager); 
}