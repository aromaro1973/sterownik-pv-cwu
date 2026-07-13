#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//==================================================
// Sterownik Nadwyżki PV Off-Grid (Hybrydowy EMS)
// Konfiguracja projektu
// Wersja: V0.3 (Sterowanie Fazowe)
// Autor: Arkadiusz Marek
//==================================================

// ---------- Wersja programu ----------
constexpr char FW_VERSION[] = "0.3.0";
constexpr char HW_VERSION[] = "1.0";
constexpr char DEVICE_NAME[] = "Sterownik Nadwyzki PV";
constexpr char AUTHOR[] = "Arkadiusz Marek";

// ---------- Piny ESP32 ----------

// Detektor przejścia przez zero (H11AA1)
constexpr uint8_t PIN_ZERO_CROSS = 27;

// Wyjście na Optotriak Random-Phase (MOC3023 -> Triak BTA24/BTA41)
constexpr uint8_t PIN_TRIAC = 14;

// LED statusowy na płytce ESP32
constexpr uint8_t PIN_LED = 2;

// ---------- Panel sterowania (Przyciski) ----------

constexpr uint8_t PIN_BUTTON_PLUS  = 32;
constexpr uint8_t PIN_BUTTON_MODE  = 33;
constexpr uint8_t PIN_BUTTON_MINUS = 25;

// ---------- Parametry sieci AC ----------

constexpr uint16_t AC_FREQUENCY = 50;
constexpr uint16_t HALF_CYCLES_PER_SECOND = 100;
constexpr uint32_t HALF_CYCLE_DURATION_US = 10000; // Jeden półokres = 10ms (10000 us)

// ---------- Limity Bezpieczeństwa (Guardian) ----------

constexpr uint32_t MAX_INVERTER_POWER_W = 3500; // Stały limit obciążenia falownika (4.2kW max)
constexpr uint32_t MAX_BATTERY_DRAW_W  = 400;  // Maksymalny pobór z akumulatora 24V dla grzałki

// ---------- Zakres regulacji mocy grzałki ----------

constexpr uint8_t MIN_POWER = 0;
constexpr uint8_t MAX_POWER = 100;
constexpr uint8_t PHASE_CONTROL_MAX_LIMIT = 40; // Górna granica bezpiecznego cięcia sinusoidy

// ---------- Komunikacja i Debug ----------

constexpr bool DEBUG_MODE = true;
constexpr uint32_t SERIAL_BAUDRATE = 115200;
constexpr uint16_t LOG_INTERVAL = 1000;

// ---------- Sieć WiFi (Dla ESP-NOW / Monitoringu) ----------

constexpr char WIFI_SSID[] = "solidarnosc";
constexpr char WIFI_PASSWORD[] = "56315236";

// ---------- Tryby pracy sterownika ----------
enum class WorkMode {
    OFF,    // Grzałka całkowicie wyłączona
    AUTO,   // Automatyczne śledzenie nadwyżek (Autocontroller + Guardian)
    MANUAL  // Ręczne ustawienie mocy przez użytkownika
};

#endif // CONFIG_H