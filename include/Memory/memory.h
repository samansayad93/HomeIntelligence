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
bool loadMotionDetection();
bool saveMotionDetection(bool enabled);

#endif
