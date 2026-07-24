#include "Guardian.h"

Guardian::Guardian() 
    : m_isBlocked(false), 
      m_blockReason(BlockReason::NONE), 
      m_maxInverterPowerW(2000), 
      m_powerStep(200),
      m_maxBatteryDrawW(400),
    m_nominalHeaterPowerW(2000),
    m_pvHoldDelayMs(600) {}

void Guardian::begin(uint16_t nominalHeaterPower) {
    m_nominalHeaterPowerW = nominalHeaterPower;
    m_isBlocked = false;
    m_blockReason = BlockReason::NONE;
}

void Guardian::update(int32_t currentInverterPowerW, uint8_t currentTriacPercent) {
    // 1. Krytyczne przekroczenie bezwzględnej mocy maksymalnej falownika
    if (currentInverterPowerW > m_maxInverterPowerW) {
        m_isBlocked = true;
        m_blockReason = BlockReason::OVERLOAD;
        return;
    }

    // 2. Szacowanie mocy domu bez uwzględnienia aktualnego wysterowania grzałki
    int32_t estimatedHeaterPower = (currentTriacPercent * m_nominalHeaterPowerW) / 100;
    int32_t powerWithoutHeater = currentInverterPowerW - estimatedHeaterPower;

    // Jeżeli domowe odbiorniki (bez grzałki) zbliżają się do limitu inwertera na odległość kroku bezpieczeństwa
    if (powerWithoutHeater > (m_maxInverterPowerW - m_powerStep)) {
        m_isBlocked = true;
        m_blockReason = BlockReason::OVERLOAD;
        return;
    }

    // Jeśli wszystko w normie i brak blokady, czyścimy stan dynamiczny
    m_isBlocked = false;
    m_blockReason = BlockReason::NONE;
}

bool Guardian::isBlocked() const {
    return m_isBlocked;
}

BlockReason BlockReason_get(const Guardian& g) {
    return g.getBlockReason();
}

BlockReason Guardian::getBlockReason() const {
    return m_blockReason;
}

void Guardian::resetBlock() {
    m_isBlocked = false;
    m_blockReason = BlockReason::NONE;
}

void Guardian::setMaxPower(uint16_t maxPower) {
    m_maxInverterPowerW = maxPower;
}

uint16_t Guardian::getMaxPower() const {
    return m_maxInverterPowerW;
}

void Guardian::setPowerStep(uint16_t step) {
    m_powerStep = step;
}

uint16_t Guardian::getPowerStep() const {
    return m_powerStep;
}

void Guardian::setMaxBatteryDraw(uint16_t maxBatteryDrawW) {
    m_maxBatteryDrawW = maxBatteryDrawW;
}

uint16_t Guardian::getMaxBatteryDraw() const {
    return m_maxBatteryDrawW;
}

void Guardian::setNominalHeaterPower(uint16_t nominalHeaterPower) {
    m_nominalHeaterPowerW = nominalHeaterPower;
}

uint16_t Guardian::getNominalHeaterPower() const {
    return m_nominalHeaterPowerW;
}

void Guardian::setPvHoldDelay(uint16_t holdDelayMs) {
    m_pvHoldDelayMs = holdDelayMs;
}

uint16_t Guardian::getPvHoldDelay() const {
    return m_pvHoldDelayMs;
}

// Zapis parametrów do pamięci flash
void Guardian::saveSettings() {
    Preferences prefs;
    // Otwarcie przestrzeni "guardian" w trybie zapisu/odczytu (false)
    prefs.begin("guardian", false);
    
    prefs.putUShort("maxPower", m_maxInverterPowerW);
    prefs.putUShort("powerStep", m_powerStep);
    prefs.putUShort("maxBatteryDraw", m_maxBatteryDrawW);
    prefs.putUShort("heaterPower", m_nominalHeaterPowerW);
    prefs.putUShort("pvHoldDelay", m_pvHoldDelayMs);
    
    prefs.end();
    Serial.println("[Guardian] Zapisano ustawienia bezpieczeństwa w pamięci NVS.");
}

// Odczyt parametrów z pamięci flash przy starcie
void Guardian::loadSettings() {
    Preferences prefs;
    // Otwarcie przestrzeni w trybie tylko do odczytu (true)
    prefs.begin("guardian", true);
    
    // Jeśli klucze nie istnieją (pierwsze uruchomienie), zostaną zachowane wartości domyślne
    m_maxInverterPowerW = prefs.getUShort("maxPower", m_maxInverterPowerW);
    m_powerStep = prefs.getUShort("powerStep", m_powerStep);
    m_maxBatteryDrawW = prefs.getUShort("maxBatteryDraw", m_maxBatteryDrawW);
    m_nominalHeaterPowerW = prefs.getUShort("heaterPower", m_nominalHeaterPowerW);
    m_pvHoldDelayMs = prefs.getUShort("pvHoldDelay", m_pvHoldDelayMs);
    
    prefs.end();
    Serial.printf("[Guardian] Wczytano z NVS -> MaxPower: %d W, Anty-Czajnik: %d W, Heater: %d W, PVHold: %d ms\n", 
                  m_maxInverterPowerW, m_powerStep, m_nominalHeaterPowerW, m_pvHoldDelayMs);
}