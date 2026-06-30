#include <IR/general.h>

IRSignal irSignals[MAX_SIGNALS];
size_t irSignalCount = 0;

IRrecv irReceiver(IR_RX_PIN);
IRsend irTransmitter(IR_TX_PIN);

void setupIR()
{
    irReceiver.enableIRIn();
    irTransmitter.begin();
}
