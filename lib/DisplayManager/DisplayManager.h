#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Config.h>
#include "ESPNowManager.h" // Dołączenie wymagane ze względu na referencję w update()

enum class DisplayScreen {
    MAIN,
    SET_MAX_POWER,
    SET_POWER_STEP,
    ESP_DIAG // <-- Nowy ekran w menu
};

class DisplayManager
{
public:
    DisplayManager();

    void begin();
    void update(const ESPNowManager& espNow); // Zmieniono: przekazujemy referencję do danych ESP-NOW

    void setMode(WorkMode mode);
    void setPower(uint8_t powerPercent);
    void setHeaterState(bool state);
    void setBurst(uint8_t burstCount);
    void setFrequency(float frequency);

    DisplayScreen getScreen() const;
    void setScreen(DisplayScreen screen);

    uint16_t getMenuMaxPower() const;
    void setMenuMaxPower(uint16_t power);

    uint16_t getMenuPowerStep() const;
    void setMenuPowerStep(uint16_t step);

    void forceRefresh();

private:
    void refreshDisplay(const ESPNowManager& espNow);
    void drawMainScreen();
    void drawMaxPowerScreen();
    void drawPowerStepScreen();
    void drawEspDiagScreen(const ESPNowManager& espNow); // <-- Nowa metoda rysująca

    LiquidCrystal_I2C m_lcd{0x27, 16, 2}; // Adres I2C 0x27, ekran 16x2

    WorkMode      m_mode;
    uint8_t       m_powerPercent;
    bool          m_heaterState;
    uint8_t       m_burstCount;
    float         m_frequency;
    DisplayScreen m_currentScreen;

    uint16_t      m_menuMaxPower;
    uint16_t      m_menuPowerStep;

    bool          m_refreshRequired;
    uint32_t      m_lastRefreshTime;
};

#endif // DISPLAY_MANAGER_H