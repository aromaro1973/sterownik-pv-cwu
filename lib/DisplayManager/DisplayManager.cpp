#include "DisplayManager.h"

DisplayManager::DisplayManager()
    : m_mode(WorkMode::OFF),
      m_powerPercent(0),
      m_heaterState(false),
      m_burstCount(0),
      m_frequency(0.0f),
    m_powerAverage(0.0f),
      m_currentScreen(DisplayScreen::SPLASH), // Startujemy od Splash Screenu
      m_currentSubScreen(0),
      m_currentServiceScreen(2),
      m_menuMaxPower(3500),
      m_menuPowerStep(1000),
      m_menuBatteryDraw(400),
    m_menuPvHoldDelay(600),
    m_menuHeaterPower(2000),
      m_refreshRequired(true),
      m_lastRefreshTime(0),
      m_splashScreenRendered(false),
      m_diagZc(0),
      m_diagTriggers(0),
      m_smoothedLatency(0.0f) // Inicjalizacja wygładzonej latencji
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

    // Wymuszaj ciągłe odświeżanie dla dynamicznych ekranów diagnostycznych
    if (m_currentScreen == DisplayScreen::INFO || m_currentScreen == DisplayScreen::CONFIG)
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

void DisplayManager::setPowerAverage(uint8_t powerPercent) {
    if (m_powerAverage <= 0.1f) {
        m_powerAverage = (float)powerPercent;
    } else {
        m_powerAverage = (powerPercent * 0.18f) + (m_powerAverage * 0.82f);
    }
    m_refreshRequired = true;
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

uint16_t DisplayManager::getMenuBatteryDraw() const { return m_menuBatteryDraw; }
void DisplayManager::setMenuBatteryDraw(uint16_t batteryDrawW) { if (m_menuBatteryDraw != batteryDrawW) { m_menuBatteryDraw = batteryDrawW; m_refreshRequired = true; } }

uint16_t DisplayManager::getMenuPvHoldDelay() const { return m_menuPvHoldDelay; }
void DisplayManager::setMenuPvHoldDelay(uint16_t holdDelayMs) { if (m_menuPvHoldDelay != holdDelayMs) { m_menuPvHoldDelay = holdDelayMs; m_refreshRequired = true; } }

uint16_t DisplayManager::getMenuHeaterPower() const { return m_menuHeaterPower; }
void DisplayManager::setMenuHeaterPower(uint16_t heaterPowerW) { if (m_menuHeaterPower != heaterPowerW) { m_menuHeaterPower = heaterPowerW; m_refreshRequired = true; } }

uint16_t DisplayManager::getMenuPowerStep() const { return m_menuPowerStep; }
void DisplayManager::setMenuPowerStep(uint16_t step) { if (m_menuPowerStep != step) { m_menuPowerStep = step; m_refreshRequired = true; } }

void DisplayManager::forceRefresh() { m_refreshRequired = true; m_lastRefreshTime = 0; }

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

// =========================================================================
// ZMODYFIKOWANA METODA DIAGNOSTYCZNA (Filtr EMA dla latencji)
// =========================================================================
void DisplayManager::updateDiagnostics(uint32_t zc, uint32_t triggers, uint32_t lastPacketTime)
{
    m_diagZc = zc;
    m_diagTriggers = triggers;

    // Obliczamy aktualne, surowe opóźnienie w sekundach
    float rawLatency = (millis() - lastPacketTime) / 1000.0f;
    if (rawLatency < 0.0f) rawLatency = 0.0f;

    // Filtr dolnoprzepustowy (EMA) - idealny wygładzacz dla LCD
    if (m_smoothedLatency <= 0.001f) {
        m_smoothedLatency = rawLatency; // Inicjalizacja przy pierwszym poprawnym przebiegu
    } else {
        m_smoothedLatency = (rawLatency * 0.12f) + (m_smoothedLatency * 0.88f); // 12% nowy, 88% stary pomiar
    }

    m_refreshRequired = true;
}

void DisplayManager::refreshDisplay(const ESPNowManager& espNow)
{
    switch (m_currentScreen)
    {
        case DisplayScreen::SPLASH:
            break;

        case DisplayScreen::MAIN:
            drawMainScreen(espNow);
            break;

        case DisplayScreen::INFO:
            switch (m_currentSubScreen)
            {
                case 1: drawEspNowRadioScreen(espNow); break;
                case 2: drawZeroCrossScreen(); break;
                case 3: drawPhaseManagerScreen(); break;
                case 4: drawAutoControllerScreen(); break;
                case 5: drawHeaterPowerScreen(); break;
                case 6: drawInverterPowerScreen(espNow); break;
                case 7: drawBatteryPowerScreen(espNow); break;
                case 8: drawPvPowerScreen(espNow); break;
                default: drawEspNowRadioScreen(espNow); break;
            }
            break;

        case DisplayScreen::CONFIG:
            switch (m_currentServiceScreen)
            {
                case 1: drawConfigMaxPowerScreen(); break;
                case 2: drawConfigBatteryDrawScreen(); break;
                case 3: drawConfigPvHoldDelayScreen(); break;
                case 4: drawConfigHeaterPowerScreen(); break;
                default: drawConfigMaxPowerScreen(); break;
            }
            break;
    }
}

// =========================================================================
// GRUPA 1: RENDERY EKRANÓW PRACY
// =========================================================================

void DisplayManager::drawMainScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("TRYB: ");
    switch (m_mode)
    {
        case WorkMode::OFF:    m_lcd.print("OFF   "); break;
        case WorkMode::AUTO:   m_lcd.print("AUTO  "); break;
        case WorkMode::MANUAL: m_lcd.print("MANUAL"); break;
    }

    m_lcd.setCursor(0, 1);
    if (m_mode == WorkMode::MANUAL && !m_heaterState)
    {
        m_lcd.print("MOC:   ");
        m_lcd.printf("%3u%%", m_powerPercent);
        m_lcd.print(" SET");
    }
    else
    {
        m_lcd.print("GRZALKA:");
        m_lcd.print(m_heaterState ? "ON " : "OFF");
        m_lcd.printf("%3u%%", m_powerPercent);
    }
}

// =========================================================================
// GRUPA INFO
// =========================================================================

void DisplayManager::drawZeroCrossScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("ZEROCROSS");

    m_lcd.setCursor(0, 1);
    m_lcd.printf("ZC:%3u F:%4.1fHz", m_diagZc, m_frequency);
}

void DisplayManager::drawPhaseManagerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("PHASE CTRL");
    
    m_lcd.setCursor(0, 1);
    m_lcd.printf("TR:%3u ZC:%3u", m_diagTriggers, m_diagZc);
}

void DisplayManager::drawEspNowRadioScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("ESP-NOW RADIO");
    m_lcd.setCursor(0, 1);
    m_lcd.print(espNow.isConnected() ? "RADIO:OK " : "RADIO:OFF");
    m_lcd.printf(" %4ums", espNow.getLastPeriodMs());
}

void DisplayManager::drawAutoControllerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("AUTOCONTROL");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("AVG PWR:%4.0f%%", m_powerAverage);
}

void DisplayManager::drawHeaterPowerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MOC GRZANIA GRZAL");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("AVG:%4.0f%%", m_powerAverage);
}

void DisplayManager::drawInverterPowerScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MOC FALOWNIKA");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("%5uW", espNow.getInverterPower());
}

void DisplayManager::drawBatteryPowerScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MOC BATERII");
    m_lcd.setCursor(0, 1);
    int32_t batPower = espNow.getBatteryPower();
    if (batPower <= 0)
    {
        m_lcd.printf("LAD:%5dW", -batPower);
    }
    else
    {
        m_lcd.printf("ROZ:%5dW", batPower);
    }
}

void DisplayManager::drawPvPowerScreen(const ESPNowManager& espNow)
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MOC PRODUKCJI PV");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("%5uW", espNow.getPVPower());
}

// =========================================================================
// GRUPA KONFIG
// =========================================================================

void DisplayManager::drawConfigMaxPowerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MOC MAX FALOWNIK");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("%4uW MODE=OK", m_menuMaxPower);
}

void DisplayManager::drawConfigBatteryDrawScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MAX ROZLAD. BATER.");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("%4uW MODE=OK", m_menuBatteryDraw);
}

void DisplayManager::drawConfigPvHoldDelayScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("CZAS ZWLOKA PV");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("%4ums MODE=OK", m_menuPvHoldDelay);
}

void DisplayManager::drawConfigHeaterPowerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MOC GRZALKI");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("%4uW MODE=SAVE", m_menuHeaterPower);
}