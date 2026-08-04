#include <PIR/pir.h>

bool motionDetectionEnabled = false;
bool motionDetectionTriggered = false;

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
    motionDetectionTriggered = false;
    if (!saveMotionDetection(true))
    {
        Serial.println("Failed to persist motion detection");
    }

    Serial.println("Motion Detection: ON");
    publishMQTT(MQTT_Alarm_Topic, "Motion Detection: ON", true);
}

void turnOFFMotionDetection()
{
    if (!motionDetectionEnabled)
    {
        return;
    }

    motionDetectionEnabled = false;
    motionDetectionTriggered = false;
    if (!saveMotionDetection(false))
    {
        Serial.println("Failed to persist motion detection");
    }

    Serial.println("Motion Detection: OFF");
    publishMQTT(MQTT_Alarm_Topic, "Motion Detection: OFF", true);
}

void checkMotionDetection()
{
    if (!motionDetectionEnabled || motionDetectionTriggered)
    {
        return;
    }

    int motion = readPIR();
    if (motion == 1)
    {
        motionDetectionTriggered = true;
        Serial.println("Motion Detected!");
        publishMQTT(MQTT_Alarm_Topic, "Motion Detected!");
        startAlarm();
    }
}
