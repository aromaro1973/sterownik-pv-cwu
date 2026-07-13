#include <Arduino.h>
#include <WiFi.h> // <-- DODAJ TĘ LINIJKĘ
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
#include <ESPNowManager.h> // Obsługa komunikacji radiowej (Zostaje)

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
// Zmienne pomocnicze
//==================================================
uint32_t logTimer = 0;
uint32_t lastEspNowPacketTime = 0; // 7-sekundowy nieblokujący bufor braku sygnału

//==================================================
// Sekcja Setup
//==================================================
void setup() {
    Serial.begin(115200);
    
    // 1. Inicjalizacja loggera na samym początku, by monitorować start systemu
    logger.begin(SERIAL_BAUDRATE);
    logger.info(F("Sterownik Nadwyzki PV (Tryb Autonomiczny)"));
    logger.info("Wersja: " + String(FW_VERSION));
    
    // 2. Konfiguracja warstwy radiowej wyłącznie dla ESP-NOW (Bez skakania po kanałach)
    WiFi.mode(WIFI_STA); 
    WiFi.disconnect(); // Czyszczenie pozostałości profilów sieciowych z pamięci Flash
    
    // Wymuszenie pracy radia na KANALE 1 (zgodnie z konfiguracją nadajnika Anenji)
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    
    Serial.print("Mój adres MAC to: ");
    Serial.println(WiFi.macAddress());
    
    // 3. Inicjalizacja odbiornika ESP-NOW
    espNowManager.begin();
    
    // 4. Inicjalizacja algorytmu Off-Grid (Moc grzałki 2000W)
    autoController.begin(2000); 

    // 5. Inicjalizacja układów wykonawczych i peryferiów
    zeroCross.begin();   
    displayManager.begin();
    controlPanel.begin();
    
    // 6. Inicjalizacja strażnika przeciążeniowego
    guardian.begin(2000); 

    // 7. Konfiguracja początkowa wyświetlacza
    displayManager.setMode(WorkMode::OFF);
    displayManager.setPower(0);
    displayManager.setBurst(0);
    displayManager.setHeaterState(false);
    
    // Zrzucenie startowych limitów z menu wyświetlacza do modułu Guardian
    guardian.setMaxPower(displayManager.getMenuMaxPower());
    guardian.setPowerStep(displayManager.getMenuPowerStep());

    // Inicjalizacja zmiennej czasu, aby zapobiec fałszywemu wyzwoleniu timeoutu przy starcie
    lastEspNowPacketTime = millis();
    
    logger.info(F("Setup zakończony pomyślnie. System gotowy do pracy."));
}

//==================================================
// Główna pętla programu
//==================================================
//==================================================
// Główna pętla programu
//==================================================
void loop()
{
    // Aktualizacja modułu radiowego (Odbiór ramek z Anenji)
    espNowManager.update();

    // Aktualizacja modułów wejściowych i wykonawczych
    zeroCross.update();
    controlPanel.update(); 

    // Odczyt impulsów kliknięć z panelu sterowania
    bool modeClicked  = controlPanel.wasModePressed();
    bool plusClicked  = controlPanel.wasPlusPressed();
    bool minusClicked = controlPanel.wasMinusPressed();

    // Pobranie aktualnego stanu systemu
    DisplayScreen currentScreen = displayManager.getScreen();
    WorkMode mode = controlPanel.getMode();
    uint8_t power = controlPanel.getManualPower();

    // --- Diagnostyka zamrożonych danych ---
    static int16_t lastPVPower = -1;
    static uint32_t lastDataChangeTime = 0;

    // Inicjalizacja czasu przy pierwszym uruchomieniu, aby uniknąć fałszywego startu
    if (lastDataChangeTime == 0) {
        lastDataChangeTime = millis();
    }

    // Aktualizacja stanu Guardiana w pętli
    guardian.update();

    // Maszyna Stanów: Zarządzanie Menu i Pracą
    if (currentScreen == DisplayScreen::MAIN)
    {
        // ---------------------------------------------
        // EKRAN GŁÓWNY (Normalna praca urządzenia)
        // ---------------------------------------------
        if (modeClicked)
        {
            // Cykliczne przełączanie trybu pracy: OFF -> AUTO -> MANUAL -> OFF
            if (mode == WorkMode::OFF)         mode = WorkMode::AUTO;
            else if (mode == WorkMode::AUTO)   mode = WorkMode::MANUAL;
            else if (mode == WorkMode::MANUAL) mode = WorkMode::OFF;
            
            power = 0; // Bezpieczny reset mocy przy zmianie trybu
            autoController.reset(); // Reset automatyki przy wejściu/wyjściu z trybu
            controlPanel.setMode(mode);
            controlPanel.setManualPower(power);
        }

        // --- Obsługa Logiki Trybów Pracy ---
        if (mode == WorkMode::MANUAL)
        {
            // Sterowanie mocą w trybie ręcznym
            if (plusClicked && power < 100)  power += 10;
            if (minusClicked && power >= 10) power -= 10;
            controlPanel.setManualPower(power);
        }
        else if (mode == WorkMode::AUTO)
        {
            // 1. Sprawdzenie, czy doszedł fizyczny pakiet sieciowy
            if (espNowManager.isConnected())
            {
                lastEspNowPacketTime = millis();

                // Sprawdzenie, czy dane przesyłane z nadajnika "żyją" i się zmieniają
                if (espNowManager.getPVPower() != lastPVPower)
                {
                    lastPVPower = espNowManager.getPVPower();
                    lastDataChangeTime = millis(); // Rejestrujemy faktyczną zmianę wartości
                }
            }

            // 2. Weryfikacja liczników bezpieczeństwa (Cisza radiowa LUB zamrożony odczyt)
            bool radioTimeout  = (millis() - lastEspNowPacketTime > 7000);
            bool frozenTimeout = (millis() - lastDataChangeTime > 7000);

            if (radioTimeout || frozenTimeout)
            {
                // WYMUSZENIE ZMIANY TRYBU NA MANUAL I ZEROWANIE MOCY
                mode = WorkMode::MANUAL;
                power = 0;
                controlPanel.setMode(mode);
                controlPanel.setManualPower(power);
                autoController.reset();

                if (radioTimeout) {
                    logger.info(F("[WATCHDOG] Brak pakietów przez 7s! Wymuszono tryb MANUAL 0%."));
                } else {
                    logger.info(F("[WATCHDOG] Dane zamrożone przez 7s! Wymuszono tryb MANUAL 0%."));
                }
            }
            else
            {
                // Parametry prawidłowe -> Obliczanie algorytmu nadwyżki
                power = autoController.calculateOffGridPower(
                    espNowManager.getPVPower(),
                    espNowManager.getInverterPower(),
                    espNowManager.getBatteryPower(), 
                    400,                                             // Próg bezpieczeństwa rozładowania (400W)
                    guardian.isBlocked()                             // Stan blokady sprzętowej Guardiana
                );
            }
            controlPanel.setManualPower(power);
        }
        
        // Wejście do menu zaawansowanych ustawień Guardiana:
        if (mode == WorkMode::OFF && plusClicked)
        {
            displayManager.setScreen(DisplayScreen::SET_MAX_POWER);
        }

        // ==================================================
        // ABSOLUTNY NADRZĘDNY BEZPIECZNIK (Dla każdego stanu)
        // ==================================================
        if (guardian.isBlocked())
        {
            power = 0; 
            displayManager.setMode(WorkMode::OFF); 
        }
        else 
        {
            displayManager.setMode(mode);
        }
      
        displayManager.setPower(power);
        displayManager.setBurst(power);
        displayManager.setHeaterState(power > 0);
    }
    else if (currentScreen == DisplayScreen::SET_MAX_POWER)
    {
        // ---------------------------------------------
        // MENU 1: Limit maksymalnego obciążenia (Waty)
        // ---------------------------------------------
        uint16_t currentMax = displayManager.getMenuMaxPower();

        if (plusClicked && currentMax < 4000)   currentMax += 100; 
        if (minusClicked && currentMax >= 100)  currentMax -= 100; 
        displayManager.setMenuMaxPower(currentMax);

        if (modeClicked)
        {
            displayManager.setScreen(DisplayScreen::SET_POWER_STEP);
        }
    }
    else if (currentScreen == DisplayScreen::SET_POWER_STEP)
    {
        // ---------------------------------------------
        // MENU 2: Limit nagłego skoku mocy (Waty)
        // ---------------------------------------------
        uint16_t currentStep = displayManager.getMenuPowerStep();

        if (plusClicked && currentStep < 3000)  currentStep += 100;
        if (minusClicked && currentStep >= 100) currentStep -= 100;
        displayManager.setMenuPowerStep(currentStep);

        if (modeClicked)
        {
            guardian.setMaxPower(displayManager.getMenuMaxPower());
            guardian.setPowerStep(displayManager.getMenuPowerStep());
            
            logger.info(F("Zapisano nowe limity modułu Guardian."));
            
            displayManager.setScreen(DisplayScreen::MAIN);
            displayManager.forceRefresh(); 
        }
    }

    displayManager.setFrequency(zeroCross.getFrequency());

    //==================================================
    // Logger Diagnostyczny (Czas rzeczywisty - Prawdziwy status)
    //==================================================
    if (Utils::elapsed(logTimer, LOG_INTERVAL))
    {
        String logMsg = "Mode=" + String((int)mode) +
                        " Power=" + String(power) + "%" +
                        " Freq=" + String(zeroCross.getFrequency(), 1);
                        
        if (guardian.isBlocked()) 
        {
            logMsg += " [ALARM: GUARDIAN BLOCKED - OVERLOAD/STEP CRASH!]";
        }
        else 
        {
            // Radio pokazuje czystą informację o połączeniu sprzętowym
            if (espNowManager.isConnected()) {
                logMsg += " [Radio: OK]";
            } else {
                logMsg += " [Radio: DISCONNECTED]";
            }
        }

        // Zawsze loguj aktualnie posiadane dane telemetryczne
        logMsg += " PV=" + String(espNowManager.getPVPower()) + 
                  "W Bat=" + String(espNowManager.getBatteryPower()) + "W";
        
        logger.info(logMsg);
    }

    displayManager.update(espNowManager); 
}