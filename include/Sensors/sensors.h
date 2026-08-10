#ifndef SENSORS_H
#define SENSORS_H

#include <config.h>
#include <Alarm/alarm.h>
#include <LDR/ldr.h>
#include <MQ2/mq2.h>
#include <MQTT/mqtt.h>
#include <DHT/dht.h>
#include <PIR/pir.h>
#include <Memory/memory.h>
#include <IR/general.h>
#include <IR/receiver.h>
#include <IR/transmitter.h>
#include <RF/general.h>
#include <RF/receiver.h>
#include <RF/transmitter.h>
#include <LED/led.h>

void setupSensors();
void printMQ2();
void printSensors();
void handleSensors();
#endif