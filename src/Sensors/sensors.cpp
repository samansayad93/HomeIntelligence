#include <Sensors/sensors.h>

unsigned long lastSensorPrint = 0;
unsigned long mq2ReadyAt = 0;
unsigned long nextMQ2ReadAt = 0;

void setupSensors()
{
    setupMQ2();
    setupDHT();
    setupPIR();
    setupBuzzer();
    setupRF();
    setupIR();
    mq2ReadyAt = millis() + MQ2_PREHEAT_DURATION;
    nextMQ2ReadAt = mq2ReadyAt;
    loadRFSignals();
    loadIRSignals();
    setupStatusLED();
}

void printMQ2()
{
    unsigned long now = millis();

    if (mq2ReadyAt == 0)
    {
        mq2ReadyAt = now + MQ2_PREHEAT_DURATION;
        nextMQ2ReadAt = mq2ReadyAt;
        Serial.println("MQ2 warming up...");
        return;
    }

    if (now < mq2ReadyAt)
    {
        return;
    }

    if (now < nextMQ2ReadAt)
    {
        return;
    }

    int gas = readMQ2();
    if (gas > MQ2_THRESHOLD)
    {
        Serial.println("MQ2 threshold exceeded!");
        publishMQTT(MQTT_Alarm_Topic, "MQ2 threshold exceeded!");
        startAlarm();
    }
    Serial.print("MQ2=");
    Serial.println(gas);
    publishMQTT(MQTT_MQ2_TOPIC, String(gas));

    nextMQ2ReadAt += MQ2_READ_INTERVAL;
    if (now > nextMQ2ReadAt)
    {
        nextMQ2ReadAt = now + MQ2_READ_INTERVAL;
    }
}

void printSensors()
{
    if (receivingIR || receivingRF)
    {
        return;
    }

    if (millis() - lastSensorPrint < SENSOR_READ_INTERVAL)
    {
        printMQ2();
        return;
    }

    Serial.println(".............................");
    int LDR = readLDR();
    Serial.print("LDR=");
    Serial.println(LDR);
    publishMQTT(MQTT_LDR_TOPIC, String(LDR));
    float temp = readTemperature();
    Serial.print("TEMP=");
    Serial.println(temp);
    publishMQTT(MQTT_TEMP_TOPIC, isnan(temp) ? "0" : String(temp, 1));
    float humidity = readHumidity();
    Serial.print("HUM=");
    Serial.println(humidity);
    publishMQTT(MQTT_HUM_TOPIC, isnan(humidity) ? "0" : String(humidity, 1));
    int motion = readPIR();
    Serial.print("PIR=");
    Serial.println(motion);
    publishMQTT(MQTT_PIR_TOPIC, String(motion));
    Serial.println(".............................");

    lastSensorPrint = millis();
    printMQ2();
}

void handleSensors()
{
    handleRFReceive(receivingRF, receiveName);
    handleIRReceive(receivingIR, receiveName);
    updateAlarm();
    updateStatusLED();
    updatePulseLED();
    checkMotionDetection();
    printSensors();
}