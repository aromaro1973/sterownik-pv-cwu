#include "DisplayManager.h"

DisplayManager::DisplayManager()
    : m_mode(WorkMode::OFF),
      m_powerPercent(0),
      m_heaterState(false),
      m_burstCount(0),
      m_frequency(0.0f),
      m_currentScreen(DisplayScreen::MAIN),
      m_menuMaxPower(3500),
      m_menuPowerStep(1000),
      m_refreshRequired(true),
      m_lastRefreshTime(0)
{
}

void DisplayManager::begin()
{
    Wire.begin(21, 22);
    m_lcd.init();
    m_lcd.backlight();
    m_lcd.clear();
    // Do początkowego renderu w begin przesyłamy pusty/tymczasowy menedżer, 
    // pełne odświeżenie nastąpi natychmiast w pętli update().
    ESPNowManager dummy;
    refreshDisplay(dummy);
}

void DisplayManager::update(const ESPNowManager& espNow)
{
    uint32_t now = millis();

    // Jeśli jesteśmy na ekranie diagnostycznym, wymuszamy odświeżanie,
    // aby licznik sekund (czas od ostatniego pakietu) aktualizował się na bieżąco.
    if (m_currentScreen == DisplayScreen::ESP_DIAG)
    {
        m_refreshRequired = true;
    }

    // OPTYMALIZACJA: Ekran odświeża się maksymalnie raz na 300ms,
    // aby transmisja I2C nie blokowała kluczowych funkcji AC w loop().
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
void DisplayManager::setScreen(DisplayScreen screen) { m_currentScreen = screen; m_lcd.clear(); m_refreshRequired = true; }

uint16_t DisplayManager::getMenuMaxPower() const { return m_menuMaxPower; }
void DisplayManager::setMenuMaxPower(uint16_t power) { if (m_menuMaxPower != power) { m_menuMaxPower = power; m_refreshRequired = true; } }

uint16_t DisplayManager::getMenuPowerStep() const { return m_menuPowerStep; }
void DisplayManager::setMenuPowerStep(uint16_t step) { if (m_menuPowerStep != step) { m_menuPowerStep = step; m_refreshRequired = true; } }

void DisplayManager::forceRefresh() { m_refreshRequired = true; m_lastRefreshTime = 0; }

void DisplayManager::refreshDisplay(const ESPNowManager& espNow)
{
    switch (m_currentScreen)
    {
        case DisplayScreen::MAIN:
            drawMainScreen();
            break;
        case DisplayScreen::SET_MAX_POWER:
            drawMaxPowerScreen();
            break;
        case DisplayScreen::SET_POWER_STEP:
            drawPowerStepScreen();
            break;
        case DisplayScreen::ESP_DIAG:
            drawEspDiagScreen(espNow);
            break;
    }
}

void DisplayManager::drawMainScreen()
{
    // Linia 1
    m_lcd.setCursor(0, 0);
    switch (m_mode)
    {
        case WorkMode::OFF:    m_lcd.print("OFF:   "); break;
        case WorkMode::AUTO:   m_lcd.print("AUTO:  "); break;
        case WorkMode::MANUAL: m_lcd.print("MANUAL "); break;
    }

    m_lcd.setCursor(7, 0);
    m_lcd.printf("%3u%%", m_powerPercent);

    m_lcd.setCursor(12, 0);
    m_lcd.print(m_heaterState ? "  ON" : " OFF");

    // Linia 2
    m_lcd.setCursor(0, 1);
    m_lcd.printf("BUR:%03u", m_burstCount);

    m_lcd.setCursor(8, 1);
    m_lcd.printf("FR:%4.1f", m_frequency);
}

void DisplayManager::drawMaxPowerScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MENU: Max Power ");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("Limit:   %4u W ", m_menuMaxPower);
}

void DisplayManager::drawPowerStepScreen()
{
    m_lcd.setCursor(0, 0);
    m_lcd.print("MENU: Power Step");
    m_lcd.setCursor(0, 1);
    m_lcd.printf("Max Step:%4u W ", m_menuPowerStep);
}

void DisplayManager::drawEspDiagScreen(const ESPNowManager& espNow)
{
    // Linia 1: Status (OK/ERR), Licznik odebranych (Rx) oraz czas od ostatniej paczki w sekundach (np. 2s)
    m_lcd.setCursor(0, 0);
    if (espNow.isConnected()) {
        m_lcd.print("OK ");
    } else {
        m_lcd.print("ERR");
    }

    m_lcd.setCursor(4, 0);
    m_lcd.printf("Rx:%-5u", espNow.getPacketCounter());

    uint32_t timeSinceLast = (millis() - espNow.getLastPacketTime()) / 1000;
    if (timeSinceLast > 99) timeSinceLast = 99; // Stała szerokość pola tekstowego
    m_lcd.setCursor(13, 0);
    m_lcd.printf("%2us", timeSinceLast);

    // Linia 2: Pakiety Zgubione (L) oraz Interwał nadawania (P) w milisekundach
    m_lcd.setCursor(0, 1);
    uint32_t lost = espNow.getLostPackets();
    if (lost > 999) lost = 999; 
    m_lcd.printf("L:%-3u", lost);

    m_lcd.setCursor(8, 1);
    uint32_t period = espNow.getLastPeriodMs();
    if (period > 9999) period = 9999;
    m_lcd.printf("P:%4ums", period);
}