#include "Guardian.h"

Guardian::Guardian() 
    : m_isBlocked(false), 
      m_blockReason(BlockReason::NONE), 
      m_maxInverterPowerW(2000), 
      m_powerStep(200),
      m_nominalHeaterPowerW(2000) {}

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

// Zapis parametrów do pamięci flash
void Guardian::saveSettings() {
    Preferences prefs;
    // Otwarcie przestrzeni "guardian" w trybie zapisu/odczytu (false)
    prefs.begin("guardian", false);
    
    prefs.putUShort("maxPower", m_maxInverterPowerW);
    prefs.putUShort("powerStep", m_powerStep);
    
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
    
    prefs.end();
    Serial.printf("[Guardian] Wczytano z NVS -> MaxPower: %d W, Anty-Czajnik: %d W\n", 
                  m_maxInverterPowerW, m_powerStep);
}