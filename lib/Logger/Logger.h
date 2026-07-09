#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

class Logger
{
public:
    enum class Level
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    void begin(uint32_t baud = 115200);

    // Standardowe metody dla obiektów String (np. przy łączeniu zmiennych)
    void debug(const String &msg);
    void info(const String &msg);
    void warning(const String &msg);
    void error(const String &msg);

    // NOWOŚĆ: Przeciążenia metod dla pamięci FLASH (makro F())
    void debug(const __FlashStringHelper *msg);
    void info(const __FlashStringHelper *msg);
    void warning(const __FlashStringHelper *msg);
    void error(const __FlashStringHelper *msg);

    void setDebug(bool enable);

private:
    bool debugEnabled = true;

    void printPrefix(Level level);
    
    void print(Level level, const String &msg);
    void print(Level level, const __FlashStringHelper *msg);

    const char *levelToString(Level level);
};

#endif // LOGGER_H