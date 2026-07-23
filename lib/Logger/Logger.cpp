#include "Logger.h"

void Logger::begin(uint32_t baud)
{
    Serial.begin(baud);
}

void Logger::setDebug(bool enable)
{
    debugEnabled = enable;
}

void Logger::setLoggingEnabled(bool enable)
{
    loggingEnabled = enable;
}

bool Logger::isLoggingEnabled() const
{
    return loggingEnabled;
}

// ==================================================
// Implementacja dla obiektów String
// ==================================================
void Logger::debug(const String &msg) {
    if (debugEnabled && loggingEnabled) print(Level::DEBUG, msg);
}
void Logger::info(const String &msg) {
    if (loggingEnabled) print(Level::INFO, msg);
}
void Logger::warning(const String &msg) {
    if (loggingEnabled) print(Level::WARNING, msg);
}
void Logger::error(const String &msg) {
    if (loggingEnabled) print(Level::ERROR, msg);
}

// ==================================================
// Implementacja dla makra F() (FLASH)
// ==================================================
void Logger::debug(const __FlashStringHelper *msg) {
    if (debugEnabled && loggingEnabled) print(Level::DEBUG, msg);
}
void Logger::info(const __FlashStringHelper *msg) {
    if (loggingEnabled) print(Level::INFO, msg);
}
void Logger::warning(const __FlashStringHelper *msg) {
    if (loggingEnabled) print(Level::WARNING, msg);
}
void Logger::error(const __FlashStringHelper *msg) {
    if (loggingEnabled) print(Level::ERROR, msg);
}

const char *Logger::levelToString(Level level)
{
    switch (level)
    {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO ";
        case Level::WARNING: return "WARN ";
        case Level::ERROR:   return "ERROR";
    }
    return "UNKWN";
}

void Logger::printPrefix(Level level)
{
    Serial.print('[');
    Serial.print(millis());
    Serial.print(F("] [")); // Użycie F() wewnątrz loggera uwalnia RAM
    Serial.print(levelToString(level));
    Serial.print(F("] "));
}

void Logger::print(Level level, const String &msg)
{
    printPrefix(level);
    Serial.println(msg);
}

void Logger::print(Level level, const __FlashStringHelper *msg)
{
    printPrefix(level);
    Serial.println(msg);
}