#include <Arduino.h>

#include <Logger.h>
#include <Config.h>
#include <Utils.h>
#include <ZeroCross.h>
#include <BurstFire.h>
#include <HeaterOutput.h>
#include <WiFiManager.h>
#include <DisplayManager.h>

Logger logger;
ZeroCross zeroCross;
BurstFire burstFire;
HeaterOutput heaterOutput;
WiFiManager wifiManager;
DisplayManager displayManager;

uint32_t logTimer = 0;
uint32_t powerTimer = 0;

bool wifiConnectedLogged = false;

// Aktualna moc testowa
uint8_t testPower = 0;

void setup()
{
    logger.begin(SERIAL_BAUDRATE);

    logger.info("Sterownik Nadwyzki PV");
    logger.info("Test BurstFire 0-100%");
    logger.info("Uruchamianie WiFi...");

    zeroCross.begin();
    heaterOutput.begin();
    displayManager.begin();

    wifiManager.begin(WIFI_SSID, WIFI_PASSWORD);

    burstFire.setPower(testPower);

    displayManager.setPower(testPower);
    displayManager.setBurst(testPower);
    displayManager.setHeaterState(false);

    logger.info("Moc = " + String(testPower) + "%");
}

void loop()
{
    wifiManager.update();

    if (wifiManager.isConnected() && !wifiConnectedLogged)
    {
        wifiConnectedLogged = true;

        logger.info("WiFi polaczone");
        logger.info("IP: " + wifiManager.getIP());
        logger.info("RSSI: " + String(wifiManager.getRSSI()) + " dBm");
    }

    zeroCross.update();

    if (zeroCross.available())
    {
        if (burstFire.next())
        {
            heaterOutput.on();
        }
        else
        {
            heaterOutput.off();
        }
    }

    // Aktualizacja częstotliwości
    displayManager.setFrequency(zeroCross.getFrequency());

    // Zmiana mocy co 5 sekund
    if (Utils::elapsed(powerTimer, 5000))
    {
        testPower += 10;

        if (testPower > 100)
        {
            testPower = 0;
        }

        burstFire.setPower(testPower);

        displayManager.setPower(testPower);
        displayManager.setBurst(testPower);
        displayManager.setHeaterState(testPower > 0);

        logger.info("================================");
        logger.info("Moc = " + String(testPower) + "%");
    }

    if (Utils::elapsed(logTimer, LOG_INTERVAL))
    {
        logger.info(
            "HalfCycles=" + String(zeroCross.getHalfCycles()) +
            "  Freq=" + String(zeroCross.getFrequency(), 1) +
            " Hz  Signal=" +
            Utils::boolToString(zeroCross.isSignalPresent()));
    }

    displayManager.update();
}