#ifndef RF_RECEIVER_H
#define RF_RECEIVER_H

#include <RF/general.h>

bool readRFSignal(const String &name);
bool handleRFReceive(bool &receivingRF, const String &receiveName);

#endif
