#ifndef MEMORY_H
#define MEMORY_H

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <RF/general.h>
#include <IR/general.h>

bool saveRFSignals();
bool loadRFSignals();
bool saveIRSignals();
bool loadIRSignals();

RFSignal *findRFSignalByName(const String &name);
IRSignal *findIRSignalByName(const String &name);

#endif
