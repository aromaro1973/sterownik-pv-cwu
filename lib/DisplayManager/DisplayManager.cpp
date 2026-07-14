#include "DisplayManager.h"

DisplayManager::DisplayManager()
    : m_mode(WorkMode::OFF),
      m_powerPercent(0),
      m_heaterState(false),
      m_burstCount(0),
      m_frequency(0.0f),
      m_currentScreen(DisplayScreen::SPLASH), // Startujemy od Splash Screenu
      m_currentSubScreen(0),
      m_currentServiceScreen(2),
      m_menuMaxPower(3500),
      m_menuPowerStep(1000),
      m_refreshRequired(true),
      m_lastRefreshTime(0),
      m_splashScreenRendered(false),
      m_diagZc(0),          // Inicjalizacja nowych zmiennych
      m_diagTriggers(0)     // Inicjalizacja nowych zmiennych
{
}

void DisplayManager::begin()
{
    Wire.begin(21, 22);
    m_lcd.init();
    m_lcd.backlight();
    m_lcd.clear();
    
    // Na samym starcie wymuszamy Splash Screen
    showSplashScreen();
}

void DisplayManager::update(const ESPNowManager& espNow)
{
    uint32_t now = millis();

    // Wymuszaj ciągłe odświeżanie dla dynamicznych ekranów diagnostycznych w menu serwisowym
    if (m_currentScreen == DisplayScreen::SERVICE)
    {
        m_refreshRequired = true;
    }

    // Optymalizacja przesyłu I2C - odświeżanie max raz na 300ms
    if (m_refreshRequired && (now - m_lastRefreshTime >= 300))
    {
        m_lastRefreshTime = now;
        refreshDisplay(espNow);
        m_refreshRequired = false;
    }
}

void DisplayManager::setMode(WorkMode mode) {
    if (m_mode != mode) { m_mode = mode; m_refreshRequired = true; }
}
void DisplayManager::setPower(uint8_t powerPercent) {
    if (m_powerPercent != powerPercent) { m_powerPercent = powerPercent; m_refreshRequired = true; }
}
void DisplayManager::setHeaterState(bool state) {
    if (m_heaterState != state) { m_heaterState = state; m_refreshRequired = true; }
}
void DisplayManager::setBurst(uint8_t burstCount) {
    if (m_burstCount != burstCount) { m_burstCount = burstCount; m_refreshRequired = true; }
}
void DisplayManager::setFrequency(float frequency) {
    if (abs(m_frequency - frequency) > 0.05f) { m_frequency = frequency; m_refreshRequired = true; }
}

DisplayScreen DisplayManager::getScreen() const { return m_currentScreen; }
void DisplayManager::setScreen(DisplayScreen screen) { 
    if (m_currentScreen != screen) {
        m_currentScreen = screen; 
        m_lcd.clear(); 
        m_refreshRequired = true; 
    }
}

void DisplayManager::setSubScreen(uint8_t subScreen) {
    if (m_currentSubScreen != subScreen) {
        m_currentSubScreen = subScreen;
        m_lcd.clear();
        m_refreshRequired = true;
    }
}
uint8_t DisplayManager::getSubScreen() const { return m_currentSubScreen; }

void DisplayManager::setServiceScreen(uint8_t serviceScreen) {
    if (m_currentServiceScreen != serviceScreen) {
        m_currentServiceScreen = serviceScreen;
        m_lcd.clear();
        m_refreshRequired = true;
    }
}
uint8_t DisplayManager::getServiceScreen() const { return m_currentServiceScreen; }

uint16_t DisplayManager::getMenuMaxPower() const { return m_menuMaxPower; }
void DisplayManager::setMenuMaxPower(uint16_t power) { if (m_menuMaxPower != power) { m_menuMaxPower = power; m_refreshRequired = true; } }

uint16_t DisplayManager::getMenuPowerStep() const { return m_menuPowerStep; }
void DisplayManager::setMenuPowerStep(uint16_t step) { if (m_menuPowerStep != step) { m_menuPowerStep = step; m_refreshRequired = true; } }

void DisplayManager::forceRefresh() { m_refreshRequired = true; m_lastRefreshTime = 0; }

// Metoda wywoływana jednorazowo podczas startu
void DisplayManager::showSplashScreen()
{
    if (!m_splashScreenRendered)
    {
        m_lcd.clear();
        m_lcd.setCursor(0, 0);
        m_lcd.print("  STEROWNIK 2.01 ");
        m_lcd.setCursor(0, 1);
        m_lcd.print("URUCHAMIANIE... ");
        m_splashScreenRendered = true;
    }
}

// Metoda aktualizująca statystyki diagnostyczne z poziomu pętli loop()
void DisplayManager::updateDiagnostics(uint32_t zc, uint32_t triggers)
{
    if (m_diagZc != zc || m_diagTriggers != triggers)
    {
        m_diagZc = zc;
        m_diagTriggers = triggers;
        m_refreshRequired = true;
    }
}

void DisplayManager::refreshDisplay(const ESPNowManager& espNow)
{
    switch (m_currentScreen)
    {
        case DisplayScreen::SPLASH:
            break;

        case DisplayScreen::MAIN:
            switch (m_currentSubScreen)
            {
                case 0: drawMainScreen(espNow); break;      // Ekran 1.0
                case 1: drawPvPowerScreen(espNow); break;    // Ekran 1.1
                case 2: drawInverterScreen(espNow); break;   // Ekran 1.2
                case 3: drawBatteryScreen(espNow); break;    // Ekran 1.3
            }
            break;

        case DisplayScreen::SERVICE:
            switch (m_currentServiceScreen)
            {
                case 2: drawZeroCrossScreen(); break;
                case 3: drawPhaseManagerScreen(); break;
                case 4: drawGuardianMaxPowerScreen(); break;
                case 5: drawGuardianDeltaPScreen(); break;
                case 6: drawEspNowRadioScreen(espNow); break;
                case 7: drawAutoControllerScreen(); break;
            }
            break;
    }
}

// =========================================================================
// GRUPA 1: RENDERY EKRANÓW PRACY (Triak aktywny, timeout 30s)
// =========================================================================

void DisplayManager::drawMainScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    switch (m_mode)
    {
        case WorkMode::OFF:    m_lcd.print("OFF   "); break;
        case WorkMode::AUTO:   m_lcd.print("AUTO  "); break;
        case WorkMode::MANUAL: m_lcd.print("MANU  "); break;
    }

    m_lcd.setCursor(6, 0);
    m_lcd.printf("%3u%%", m_powerPercent);

    m_lcd.setCursor(12, 0);
    m_lcd.print(m_heaterState ? "  ON" : " OFF");

    m_lcd.setCursor(0, 1);
    m_lcd.print("RADIO: ");
    m_lcd.print(espNow.isConnected() ? "OK      " : "ERR     ");
}

void DisplayManager::drawPvPowerScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("INFO: POMIARY PV");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("MOC PV: %5uW", espNow.getPVPower());
}

void DisplayManager::drawInverterScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("INFO: INWERTER  ");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("DOM/INV:%5uW", espNow.getInverterPower());
}

void DisplayManager::drawBatteryScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("INFO: AKUMULATOR");
    m_lcd.setCursor(0, 1);
    
    int32_t batPower = espNow.getBatteryPower();
    if (batPower <= 0)
    {
        m_lcd.printf("BAT: LAD. %4dW", batPower);
    }
    else
    {
        m_lcd.printf("BAT: ROZ. +%3dW", batPower);
    }
}

// =========================================================================
// GRUPA 2: RENDERY MENU SERWISOWEGO (Grzałka wyłączona!)
// =========================================================================

void DisplayManager::drawZeroCrossScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MOD: ZeroCross ");
    m_lcd.print(m_frequency > 45.0f ? "OK" : "ERR");

    m_lcd.setCursor(0, 1);
    // Wyświetlanie PRAWDZIWYCH impulsów przejścia przez zero na sekundę (m_diagZc) zamiast sztywnych 100/0
    m_lcd.printf("ZC/s:%3u F:%4.1fH", m_diagZc, m_frequency);
}

void DisplayManager::drawPhaseManagerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MOD: PhaseCtr ");
    m_lcd.print(m_diagTriggers > 0 ? "ACT" : "OFF");
    
    m_lcd.setCursor(0, 1);
    // Wyświetlanie PRAWDZIWYCH wyzwoleń triaka na sekundę zliczonych przez Core 1
    m_lcd.printf("Trig/s: %3u/100", m_diagTriggers);
}

void DisplayManager::drawGuardianMaxPowerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("GUARDIAN STAT:ON");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("Inv Max:  %4uW", m_menuMaxPower);
}

void DisplayManager::drawGuardianDeltaPScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("GUARDIAN DYNC:ON");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("Max dP:   %4uW", m_menuPowerStep);
}

void DisplayManager::drawEspNowRadioScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("NADAJNIK: RAD: ");
    m_lcd.print(espNow.isConnected() ? "OK " : "ERR");

    m_lcd.setCursor(0, 1);
    uint32_t avgPeriod = espNow.getLastPeriodMs();
    if (avgPeriod > 9999) avgPeriod = 9999;
    m_lcd.printf("Avg Period:%3ums", avgPeriod);
}

void DisplayManager::drawAutoControllerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("AUTOCONTROL: ON ");
    m_lcd.setCursor(0, 1);
    m_lcd.print("EMS Loop: Active");
}