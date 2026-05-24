#include <RF/general.h>

RFSignal signals[MAX_SIGNALS];
size_t signalCount = 0;

RCSwitch mySwitch = RCSwitch();

void setupRF(){
    mySwitch.enableReceive(RF_RX_PIN);
    mySwitch.enableTransmit(RF_TX_PIN);
}