#include <Arduino.h>

#include <Logger.h>
#include <Config.h>
#include <Utils.h>
#include <ZeroCross.h>
#include <WiFiManager.h>
#include <DisplayManager.h>
#include <ControlPanel.h>
#include <Guardian.h>
#include <AutoController.h>
#include <PhaseController.h>
#include <ESPNowManager.h> // Obsługa komunikacji radiowej
#include <HAManager.h>

//==================================================
// Instancje obiektów globalnych
//==================================================
Logger logger;
ZeroCross zeroCross;
PhaseController phaseController;
WiFiManager wifiManager;
DisplayManager displayManager;
ControlPanel controlPanel;
Guardian guardian;
ESPNowManager espNowManager;
AutoController autoController;

// KLUCZOWA POPRAWKA: Przeniesione PONIŻEJ obiektów, od których zależy HAManager
WiFiClient espClient;
HAManager haManager(espClient, espNowManager, guardian, controlPanel);

//==================================================
// Zmienne pomocnicze
//==================================================
uint32_t logTimer = 0;
bool wifiConnectedLogged = false;

// ... dalej kod bez zmian (setup i loop) ...

//==================================================
// Sekcja Setup
//==================================================
void setup() {
    Serial.begin(115200);
    
    // Konfiguracja trybu WiFi oraz odczyt adresu MAC dla drugiego ESP
    WiFi.mode(WIFI_STA); 
    Serial.print("Mój adres MAC to: ");
    Serial.println(WiFi.macAddress());
    
    // Inicjalizacja odbiornika ESP-NOW
    espNowManager.begin();
    
    // Inicjalizacja algorytmu Off-Grid (Podaj realną moc grzałki, np. 2000W)
    autoController.begin(2000); 

    // Inicjalizacja portu szeregowego
    logger.begin(SERIAL_BAUDRATE);

    // Komunikaty startowe (teksty statyczne przeniesione do pamięci FLASH)
    logger.info(F("Sterownik Nadwyzki PV"));
    logger.info("Wersja: " + String(FW_VERSION));

    // Inicjalizacja układów wykonawczych i peryferiów
    zeroCross.begin();   
    displayManager.begin();
    controlPanel.begin();
    
    // Inicjalizacja strażnika (podajemy domyślną moc grzałki)
    guardian.begin(2000); 

    // Start połączenia WiFi w tle (nieblokujący)
    wifiManager.begin(WIFI_SSID, WIFI_PASSWORD);

    // Po zainicjalizowaniu wifiManager.begin(...)
    haManager.begin("111.222.33.44", 1883, "username_mqtt", "haslo_mqtt"); // Podaj IP swojego brokera Mosquitto w HA

  

    // Konfiguracja początkowa wyświetlacza
    displayManager.setMode(WorkMode::OFF);
    displayManager.setPower(0);
    displayManager.setBurst(0);
    displayManager.setHeaterState(false);
    
    // Zrzucenie startowych limitów z menu wyświetlacza do modułu Guardian
    guardian.setMaxPower(displayManager.getMenuMaxPower());
    guardian.setPowerStep(displayManager.getMenuPowerStep());
}

//==================================================
// Główna pętla programu
//==================================================
void loop()
{
    // Aktualizacja modułu radiowego (Watchdog sygnału)
    espNowManager.update();

    // Obsługa połączenia sieciowego
    wifiManager.update();

    // Logowanie statusu sieci (wykona się tylko raz po udanym połączeniu)
    if (wifiManager.isConnected() && !wifiConnectedLogged)
    {
        wifiConnectedLogged = true;
        logger.info(F("WiFi polaczone"));
        logger.info("IP: " + wifiManager.getIP());
        logger.info("RSSI: " + String(wifiManager.getRSSI()) + " dBm");
    }

    // Aktualizacja modułów wejściowych
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

    // ==================================================
    // KROK 1: Aktualizacja stanu Guardiana w pętli
    // ==================================================
    // Guardian potrzebuje wiedzieć, jaki poziom mocy aktualnie przetwarzamy
    guardian.update();

    haManager.update();

    //==================================================
    // Maszyna Stanów: Zarządzanie Menu i Pracą
    //==================================================
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
            autoController.reset(); // Resetujemy automatykę przy wejściu/wyjściu z trybu
            controlPanel.setMode(mode);
            controlPanel.setManualPower(power);
        }

        // --- Obsługa Logiki Trybów Pracy ---
        if (mode == WorkMode::MANUAL)
        {
            // Sterowanie mocą w trybie ręcznym (Działa zawsze, bez względu na radio)
            if (plusClicked && power < 100)  power += 10;
            if (minusClicked && power >= 10) power -= 10;
            controlPanel.setManualPower(power);
        }
        else if (mode == WorkMode::AUTO)
        {
            // Inteligentna automatyka Off-Grid oparta na danych radiowych z Anenji
            if (espNowManager.isConnected())
            {
                power = autoController.calculateOffGridPower(
                    espNowManager.getPVPower(),
                    espNowManager.getInverterPower(),
                    espNowManager.getBatteryPower(), 
                    400,                             // Próg bezpieczeństwa rozładowania (400W)
                    guardian.isBlocked()             // Stan blokady sprzętowej Guardiana
                );
            }
            else
            {
                // Awaria radia w trybie AUTO -> bezpieczne, automatyczne odcięcie grzałki
                power = 0;
            }
            controlPanel.setManualPower(power);
        }
        
        // Wejście do menu zaawansowanych ustawień Guardiana:
        if (mode == WorkMode::OFF && plusClicked)
        {
            displayManager.setScreen(DisplayScreen::SET_MAX_POWER);
        }

        // ==================================================
        // KROK 2: ABSOLUTNY NADRZĘDNY BEZPIECZNIK (Dla każdego stanu)
        // ==================================================
        if (guardian.isBlocked())
        {
            power = 0; // Wymuszony reset mocy - blokada przeciążeniowa/skokowa
            displayManager.setMode(WorkMode::OFF); // Nadpisanie widoku dla ekranu LCD 2x16
        }
        else 
        {
            displayManager.setMode(mode);
        }

        // Przekazanie aktualnych nastawów do modułów wykonawczych i LCD
      
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
    // Logger Diagnostyczny (Rozszerzony o precyzyjne stany)
    //==================================================
    if (Utils::elapsed(logTimer, LOG_INTERVAL))
    {
        String logMsg = "Mode=" + String((int)mode) +
                        " Power=" + String(power) + "%" +
                        " Freq=" + String(zeroCross.getFrequency(), 1);
                        
        // Sprawdzenie krytycznej blokady nadrzędnej
        if (guardian.isBlocked()) 
        {
            logMsg += " [ALARM: GUARDIAN BLOCKED - OVERLOAD/STEP CRASH!]";
        }
        // Analiza stanu komunikacji radiowej
        else if (espNowManager.isConnected()) 
        {
            logMsg += " [Radio: OK] PV=" + String(espNowManager.getPVPower()) + 
                      "W Bat=" + String(espNowManager.getBatteryPower()) + "W";
        } 
        else 
        {
            if (mode == WorkMode::AUTO) {
                logMsg += " [Radio: DISCONNECTED - AUTO STOPPED]";
            } else {
                logMsg += " [Radio: DISCONNECTED - MANUAL OFFLINE RUN]";
            }
        }
        
        logger.info(logMsg);
    }

    // Wyczyszczono podwójny średnik z końca linii
    displayManager.update(espNowManager); 
}