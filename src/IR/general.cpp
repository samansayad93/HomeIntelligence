#include <IR/general.h>

IRSignal irSignals[MAX_SIGNALS];
size_t irSignalCount = 0;

IRrecv irReceiver(IR_RX_PIN, 1024, 50, true);
IRsend irTransmitter(IR_TX_PIN);

void setupIR()
{
    irReceiver.enableIRIn();
    irTransmitter.begin();
}
