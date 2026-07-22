#include <PIR/pir.h>

bool motionDetectionEnabled = false;

void setupPIR()
{
    pinMode(PIR_PIN, INPUT);

    motionDetectionEnabled = loadMotionDetection();
    if (motionDetectionEnabled)
    {
        Serial.println("Motion detection restored to ON");
    }
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
    if (motionDetectionEnabled)
    {
        return;
    }

    motionDetectionEnabled = true;
    if (!saveMotionDetection(true))
    {
        Serial.println("Failed to persist motion detection");
    }
}

void turnOFFMotionDetection()
{
    if (!motionDetectionEnabled)
    {
        return;
    }

    motionDetectionEnabled = false;
    if (!saveMotionDetection(false))
    {
        Serial.println("Failed to persist motion detection");
    }
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
        Serial.println("Motion Detected!");
        publishMQTT(MQTT_Alarm_Topic, "Motion Detected!");
        startAlarm();
    }
}
