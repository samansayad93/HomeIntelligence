#include <DHT/dht.h>

DHT dht(DHT_PIN, DHT_TYPE);

void setupDHT()
{
    dht.begin();
}

float readHumidity()
{
    float input = dht.readHumidity();
    if (isnan(input))
    {
        Serial.println("Failed to read humidity from DHT sensor");
    }
    return input;
}

float readTemperature()
{
    float input = dht.readTemperature();
    if (isnan(input))
    {
        Serial.println("Failed to read temperature from DHT sensor");
    }
    return input;
}
