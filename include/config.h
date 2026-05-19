#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const char FILE_PATH[] = "/signals.json";
const size_t MAX_SIGNALS = 20;

#define LDR_PIN 3

#define DHT_PIN 4
#define DHT_TYPE DHT11

#define PIR_PIN 5

#define RF_RX_PIN 6
#define RF_TX_PIN 7

#endif