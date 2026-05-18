#ifndef DHT_SELF_H
#define DHT_SELF_H

#include <config.h>
#include <DHT.h>

extern DHT dht;

void setupDHT();
float readHumidity();
float readTemperature();

#endif