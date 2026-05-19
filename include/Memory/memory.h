#ifndef MEMORY_H
#define MEMORY_H

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <RF/general.h>

bool saveRFSignals();
bool loadRFSignals();

#endif