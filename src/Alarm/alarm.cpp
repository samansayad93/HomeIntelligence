#include <Alarm/alarm.h>

bool alarmActive = false;
bool buzzerOn = false;
unsigned long lastBuzzerToggle = 0;

void setupBuzzer()
{
    pinMode(BUZZER_PIN, OUTPUT);
    stopAlarm();
}

void startAlarm()
{
    alarmActive = true;
    buzzerOn = true;
    lastBuzzerToggle = millis();
    digitalWrite(BUZZER_PIN, HIGH);
}

void stopAlarm()
{
    alarmActive = false;
    buzzerOn = false;
    digitalWrite(BUZZER_PIN, LOW);
}

void updateAlarm()
{
    if (!alarmActive)
    {
        return;
    }

    if (millis() - lastBuzzerToggle < BUZZER_DUTY_CYCLE_INTERVAL)
    {
        return;
    }

    buzzerOn = !buzzerOn;
    lastBuzzerToggle = millis();
    digitalWrite(BUZZER_PIN, buzzerOn ? HIGH : LOW);
}
