#ifndef RF_GENERAL_H
#define RF_GENERAL_H

#include <config.h>
#include <RCSwitch.h>

struct RFSignal
{
    String name;
    uint32_t code;
    uint16_t bits;
    uint16_t protocol;
    uint16_t pulse;
};

extern RCSwitch mySwitch;

extern RFSignal signals[];
extern size_t signalCount;

void setupRF();
void listRFSignals();

#endif