#ifndef RF_GENERAL_H
#define RF_GENERAL_H

#include <config.h>
#include <RCSwitch.h>
#include <ArduinoJson.h>
#include <MQTT/mqtt.h>
#include <LED/led.h>

struct RFSignal
{
    String name;
    uint32_t code;
    uint16_t bits;
    uint16_t protocol;
    uint16_t pulse;
    uint16_t band; // 433 or 315
};

extern RCSwitch rfSwitch433;
extern RCSwitch rfSwitch315;

extern RFSignal signals[];
extern size_t signalCount;

RCSwitch *getRFSwitch(uint16_t band);

void setupRF();
void listRFSignals();
RFSignal *findRFSignalByName(const String &name);

#endif