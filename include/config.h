#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const char RF_FILE_PATH[] = "/rf_signals.json";
const char IR_FILE_PATH[] = "/ir_signals.json";
const size_t MAX_SIGNALS = 20;
const size_t MAX_IR_RAW_LENGTH = 200;

#define LDR_PIN 34
#define MQ2_PIN 35

#define DHT_PIN 4
#define DHT_TYPE DHT11

#define PIR_PIN 5

#define RF_RX_PIN 27
#define RF_TX_PIN 26

#define IR_RX_PIN 14
#define IR_TX_PIN 25

#define SENSOR_READ_INTERVAL 2000
#define MQ2_PREHEAT_DURATION 30000
#define MQ2_READ_INTERVAL 2000

#endif
