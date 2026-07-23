#ifndef IR_GENERAL_H
#define IR_GENERAL_H

#include <config.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <ArduinoJson.h>
#include <MQTT/mqtt.h>

struct IRSignal
{
    String name;
    decode_type_t protocol;
    uint64_t value;
    uint16_t bits;
    uint16_t rawLength;
    uint16_t rawData[MAX_IR_RAW_LENGTH];
};

extern IRrecv irReceiver;
extern IRsend irTransmitter;

extern IRSignal irSignals[];
extern size_t irSignalCount;

void setupIR();
void listIRSignals();

#endif
