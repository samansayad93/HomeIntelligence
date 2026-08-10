#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

const char PROVISIONING_FILE_PATH[] = "/provisioning.json";
const char PROVISIONING_AP_SSID[] = "Intelligence-Setup";
const char PROVISIONING_AP_PASSWORD[] = "12345678";

const char RF_FILE_PATH[] = "/rf_signals.json";
const char IR_FILE_PATH[] = "/ir_signals.json";
const char SETTINGS_FILE_PATH[] = "/settings.json";
const size_t MAX_SIGNALS = 20;
const size_t MAX_IR_RAW_LENGTH = 200;

const char WIFI_SSID[] = "YOUR_WIFI_SSID";
const char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";

const char MQTT_HOST[] = "192.168.1.10";
const uint16_t MQTT_PORT = 1883;
const char MQTT_CLIENT_ID[] = "esp32-intelligence";
const char MQTT_USER[] = "";
const char MQTT_PASSWORD[] = "";
const char MQTT_BASE_TOPIC[] = "homeintelligence";

const char MQTT_LDR_TOPIC[] = "sensor/LDR";
const char MQTT_TEMP_TOPIC[] = "sensor/TEMP";
const char MQTT_HUM_TOPIC[] = "sensor/HUM";
const char MQTT_PIR_TOPIC[] = "sensor/PIR";
const char MQTT_MQ2_TOPIC[] = "sensor/MQ2";
const char MQTT_Alarm_Topic[] = "alarm";
const char MQTT_RF_SIGNAL_TOPIC[] = "RF/list";
const char MQTT_IR_SIGNAL_TOPIC[] = "IR/list";

extern String receiveName;
extern bool receivingRF;
extern bool receivingIR;

#define MQTT_RECONNECT_INTERVAL 1000

#define WIFI_PROVISIONING_FALLBACK_MS 30000

#define LDR_PIN 34

#define MQ2_PIN 35

#define DHT_PIN 33
#define DHT_TYPE DHT11

#define PIR_PIN 13

#define BUZZER_PIN 32
#define STATUS_LED_PIN 12

#define RF_RX_433_PIN 27
#define RF_TX_433_PIN 26
#define RF_RX_315_PIN 4
#define RF_TX_315_PIN 5

#define IR_RX_PIN 14
#define IR_TX_PIN 25

#define SENSOR_READ_INTERVAL 10000
#define MQ2_PREHEAT_DURATION 300000
#define MQ2_READ_INTERVAL 10000
#define MQ2_THRESHOLD 700

#define BUZZER_DUTY_CYCLE_INTERVAL 2000

#define STATUS_LED_ON 1000
#define STATUS_LED_OFF 5000
#define STATUS_LED_PULSE 500

#endif
