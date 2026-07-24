#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Config.h>
#include "ESPNowManager.h"

enum class DisplayScreen {
    SPLASH,          
    MAIN,            
    INFO,
    CONFIG          
};

class DisplayManager
{
public:
    DisplayManager();

    void begin();
    void update(const ESPNowManager& espNow);

    void setMode(WorkMode mode);
    void setPower(uint8_t powerPercent);
    void setPowerAverage(uint8_t powerPercent);
    void setHeaterState(bool state);
    void setBurst(uint8_t burstCount);
    void setFrequency(float frequency);

    DisplayScreen getScreen() const;
    void setScreen(DisplayScreen screen);

    void setSubScreen(uint8_t subScreen);
    uint8_t getSubScreen() const;

    void setServiceScreen(uint8_t serviceScreen);
    uint8_t getServiceScreen() const;

    uint16_t getMenuMaxPower() const;
    void setMenuMaxPower(uint16_t power);

    uint16_t getMenuBatteryDraw() const;
    void setMenuBatteryDraw(uint16_t batteryDrawW);

    uint16_t getMenuPvHoldDelay() const;
    void setMenuPvHoldDelay(uint16_t holdDelayMs);

    uint16_t getMenuHeaterPower() const;
    void setMenuHeaterPower(uint16_t heaterPowerW);

    uint16_t getMenuPowerStep() const;
    void setMenuPowerStep(uint16_t step);

    void forceRefresh();
    void showSplashScreen(); 

    // =========================================================================
    // Zaktualizowana Metoda diagnostyczna (przyjmuje teraz czas ostatniego pakietu)
    // =========================================================================
    void updateDiagnostics(uint32_t zc, uint32_t triggers, uint32_t lastPacketTime);

private:
    void refreshDisplay(const ESPNowManager& espNow);

    void drawMainScreen(const ESPNowManager& espNow); 
    void drawZeroCrossScreen(); 
    void drawPhaseManagerScreen(); 
    void drawEspNowRadioScreen(const ESPNowManager& espNow); 
    void drawAutoControllerScreen(); 
    void drawHeaterPowerScreen();
    void drawInverterPowerScreen(const ESPNowManager& espNow);
    void drawBatteryPowerScreen(const ESPNowManager& espNow);
    void drawPvPowerScreen(const ESPNowManager& espNow);
    void drawConfigMaxPowerScreen();
    void drawConfigBatteryDrawScreen();
    void drawConfigPvHoldDelayScreen();
    void drawConfigHeaterPowerScreen();

    LiquidCrystal_I2C m_lcd{0x27, 16, 2}; 

    WorkMode      m_mode;
    uint8_t       m_powerPercent;
    bool          m_heaterState;
    uint8_t       m_burstCount;
    float         m_frequency;
    float         m_powerAverage;
    
    DisplayScreen m_currentScreen;
    uint8_t       m_currentSubScreen;     
    uint8_t       m_currentServiceScreen; 

    uint16_t      m_menuMaxPower;
    uint16_t      m_menuPowerStep;
    uint16_t      m_menuBatteryDraw;
    uint16_t      m_menuPvHoldDelay;
    uint16_t      m_menuHeaterPower;

    bool          m_refreshRequired;
    uint32_t      m_lastRefreshTime;
    bool          m_splashScreenRendered; 

    // Statystyki diagnostyczne (Nowy Ekran 1.4)
    uint32_t      m_diagZc;       // Przejścia przez zero / sekundę
    uint32_t      m_diagTriggers; // Wyzwolenia triaka / sekundę
    float         m_smoothedLatency; // Uśredniona latencja ESP-NOW (EMA)
};

#endif // DISPLAY_MANAGER_H