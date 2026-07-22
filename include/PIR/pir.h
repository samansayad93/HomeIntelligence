#ifndef PIR_H
#define PIR_H

#include <config.h>
#include <Alarm/alarm.h>
#include <MQTT/mqtt.h>
#include <Memory/memory.h>

void setupPIR();
int readPIR();
void turnONMotionDetection();
void turnOFFMotionDetection();
void checkMotionDetection();

#endif