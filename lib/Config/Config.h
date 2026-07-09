#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//==================================================
// Sterownik Nadwyżki PV
// Konfiguracja projektu
// Wersja: V0.2
// Autor: Arkadiusz Marek
//==================================================

// ---------- Wersja programu ----------
constexpr char FW_VERSION[] = "0.2.0";

// ---------- Piny ESP32 ----------

// H11AA1
constexpr uint8_t PIN_ZERO_CROSS = 27;

// PC817 -> MOC3083
constexpr uint8_t PIN_TRIAC = 14;

// LED na płytce
constexpr uint8_t PIN_LED = 2;

// ---------- Panel sterowania ----------

constexpr uint8_t PIN_BUTTON_PLUS  = 32;
constexpr uint8_t PIN_BUTTON_MODE  = 33;
constexpr uint8_t PIN_BUTTON_MINUS = 25;


// ---------- Parametry sieci ----------

constexpr uint16_t AC_FREQUENCY = 50;
constexpr uint16_t HALF_CYCLES_PER_SECOND = 100;


// ---------- Burst Fire ----------

constexpr uint16_t BURST_WINDOW = 100;


// ---------- Zakres regulacji ----------

constexpr uint8_t MIN_POWER = 0;
constexpr uint8_t MAX_POWER = 100;

// ---------- Debug ----------

constexpr bool DEBUG_MODE = true;

// ---------- Nazwa urządzenia i autor ----------

constexpr char DEVICE_NAME[] = "Sterownik Nadwyzki PV";

constexpr char AUTHOR[] = "Arkadiusz Marek";

// ---------- Serial ----------

constexpr uint32_t SERIAL_BAUDRATE = 115200;

// ---------- Hardware version ----------

constexpr char HW_VERSION[] = "1.0";

// ---------- Log interval ----------

constexpr uint16_t LOG_INTERVAL = 1000;

// ---------- WiFi ----------

constexpr char WIFI_SSID[] = "solidarnosc";
constexpr char WIFI_PASSWORD[] = "56315235";


enum class WorkMode {
    OFF,
    AUTO,
    MANUAL
};

#endif