#include <LED/led.h>

bool statusLED = false;
bool pulseActive = false;
unsigned long lastLEDToggle = 0;
unsigned long pulseStart = 0;

void setupStatusLED()
{
    pinMode(STATUS_LED_PIN, OUTPUT);
}

void updateStatusLED()
{
    unsigned long interval = statusLED ? STATUS_LED_ON : STATUS_LED_OFF;

    if (millis() - lastLEDToggle < interval)
    {
        return;
    }

    statusLED = !statusLED;
    digitalWrite(STATUS_LED_PIN, statusLED ? HIGH : LOW);
    lastLEDToggle = millis();
}

void pulseStatusLED()
{
    digitalWrite(STATUS_LED_PIN, HIGH);
    pulseActive = true;
    pulseStart = millis();
}

void updatePulseLED()
{
    if (pulseActive && (millis() - pulseStart >= STATUS_LED_PULSE))
    {
        digitalWrite(STATUS_LED_PIN, LOW);
        pulseActive = false;
    }
}