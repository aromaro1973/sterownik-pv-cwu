#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Config.h>
#include "ESPNowManager.h"

// Nowa lista ekranów dopasowana do założeń EMS
enum class DisplayScreen {
    SPLASH,          // 1. Ekran startowy (Splash screen)
    MAIN,            // 2. Grupa 1: Ekrany pracy (zawiera podekrany 1.0 - 1.3)
    SERVICE          // 3. Grupa 2: Głębokie Menu Serwisowe (zawiera ekrany 2 - 7)
};

class DisplayManager
{
public:
    DisplayManager();

    void begin();
    void update(const ESPNowManager& espNow);

    void setMode(WorkMode mode);
    void setPower(uint8_t powerPercent);
    void setHeaterState(bool state);
    void setBurst(uint8_t burstCount);
    void setFrequency(float frequency);

    DisplayScreen getScreen() const;
    void setScreen(DisplayScreen screen);

    // Gettery i settery do nawigacji po grupach ekranów
    void setSubScreen(uint8_t subScreen);
    uint8_t getSubScreen() const;

    void setServiceScreen(uint8_t serviceScreen);
    uint8_t getServiceScreen() const;

    uint16_t getMenuMaxPower() const;
    void setMenuMaxPower(uint16_t power);

    uint16_t getMenuPowerStep() const;
    void setMenuPowerStep(uint16_t step);

    void forceRefresh();
    void showSplashScreen(); // Flaga wymuszenia natychmiastowego startu rysowania splash

private:
    void refreshDisplay(const ESPNowManager& espNow);

    // --- GRUPA 1: Metody rysujące ekrany pracy ---
    void drawMainScreen(const ESPNowManager& espNow); // Ekran 1.0
    void drawPvPowerScreen(const ESPNowManager& espNow); // Podekran 1.1
    void drawInverterScreen(const ESPNowManager& espNow); // Podekran 1.2
    void drawBatteryScreen(const ESPNowManager& espNow); // Podekran 1.3

    // --- GRUPA 2: Metody rysujące Menu Serwisowe ---
    void drawZeroCrossScreen(); // Ekran 2 (Diagnostyka ZC)
    void drawPhaseManagerScreen(); // Ekran 3 (Diagnostyka PhaseManager)
    void drawGuardianMaxPowerScreen(); // Ekran 4 (Konfiguracja Guardian Max Power)
    void drawGuardianDeltaPScreen(); // Ekran 5 (Konfiguracja Guardian Delta P)
    void drawEspNowRadioScreen(const ESPNowManager& espNow); // Ekran 6 (Diagnostyka ESP-NOW)
    void drawAutoControllerScreen(); // Ekran 7 (Diagnostyka AutoController)

    LiquidCrystal_I2C m_lcd{0x27, 16, 2}; // Adres I2C 0x27, ekran 16x2

    WorkMode      m_mode;
    uint8_t       m_powerPercent;
    bool          m_heaterState;
    uint8_t       m_burstCount;
    float         m_frequency;
    
    DisplayScreen m_currentScreen;
    uint8_t       m_currentSubScreen;     // 0 = 1.0, 1 = 1.1, 2 = 1.2, 3 = 1.3
    uint8_t       m_currentServiceScreen; // Zakres 2 - 7

    uint16_t      m_menuMaxPower;
    uint16_t      m_menuPowerStep;

    bool          m_refreshRequired;
    uint32_t      m_lastRefreshTime;
    bool          m_splashScreenRendered; // Flaga jednorazowego renderu splash
};

#endif // DISPLAY_MANAGER_H