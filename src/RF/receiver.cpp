#include <RF/receiver.h>

RCSwitch mySwitch = RCSwitch();

void setupRFRX()
{
    mySwitch.enableReceive(RF_RX_PIN);
}

void readRFRX(String name)
{
    if (mySwitch.available())
    {
        uint32_t code = mySwitch.getReceivedValue();
        uint16_t bits = mySwitch.getReceivedBitlength();
        uint16_t protocol = mySwitch.getReceivedProtocol();
        uint16_t pulse = mySwitch.getReceivedDelay();

        signals[signalCount].name = name;
        signals[signalCount].code = code;
        signals[signalCount].bits = bits;
        signals[signalCount].protocol = protocol;
        signals[signalCount].pulse = pulse;
        signalCount++;

        mySwitch.resetAvailable();
    }
}