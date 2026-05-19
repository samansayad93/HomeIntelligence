#include <config.h>
#include <LDR/ldr.h>
#include <DHT/dht.h>
#include <PIR/pir.h>

void setup()
{
    Serial.begin(115200);
    delay(200);
    setupDHT();
    setupPIR();
}

void loop()
{
    int LDR = readLDR();
    Serial.println(String(LDR));
    float temp = readTemperature();
    Serial.println(String(temp));
    float humidity = readHumidity();
    Serial.println(String(humidity));
    int motion = readPIR();
    Serial.println(String(motion));
}