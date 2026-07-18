#include <PIR/pir.h>

bool motionDetectionEnabled = false;

void setupPIR()
{
    pinMode(PIR_PIN, INPUT);
}

int readPIR()
{
    int input = digitalRead(PIR_PIN);
    if (input == HIGH)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void turnONMotionDetection()
{
    motionDetectionEnabled = true;
}

void turnOFFMotionDetection()
{
    motionDetectionEnabled = false;
}

void checkMotionDetection()
{
    if (!motionDetectionEnabled)
    {
        return;
    }

    int motion = readPIR();
    if (motion == 1)
    {
        Serial.println("Motion detected!");
        publishMQTT(MQTT_Alarm_Topic, "motion detected");
        startAlarm();
    }
}