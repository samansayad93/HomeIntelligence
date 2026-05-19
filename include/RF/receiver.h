#ifndef RF_RECEIVER_H
#define RF_RECEIVER_H

#include <config.h>
#include <RCSwitch.h>
#include <RF/general.h>

extern RCSwitch mySwitch;

void setupRFRX();
void readRFRX();

#endif