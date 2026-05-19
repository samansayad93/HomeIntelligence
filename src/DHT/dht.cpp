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
    }
    return input;
}

float readTemperature()
{
    float input = dht.readTemperature();
    if (isnan(input))
    {
    }
    return input;
}