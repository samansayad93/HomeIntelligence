#ifndef SERIAL_H
#define SERIAL_H

#include <config.h>
#include <Alarm/alarm.h>
#include <PIR/pir.h>
#include <Memory/memory.h>
#include <IR/general.h>
#include <IR/receiver.h>
#include <IR/transmitter.h>
#include <RF/general.h>
#include <RF/receiver.h>
#include <RF/transmitter.h>

void printHelp();
void handleSerialCommand(const String &command);
void handleSerial();
#endif