#ifndef IR_RECEIVER_H
#define IR_RECEIVER_H

#include <IR/general.h>

void resetIRReceiver();
bool readIRSignal(const String &name);
bool handleIRReceive(bool &receivingIR, const String &receiveName);

#endif
