#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const char FILE_PATH[] = "/signals.json";
const size_t MAX_SIGNALS = 20;

#define LDR_PIN 34

#define DHT_PIN 4
#define DHT_TYPE DHT11

#define PIR_PIN 5

#define RF_RX_PIN 27
#define RF_TX_PIN 26

#define SENSOR_READ_INTERVAL 2000

#endif
