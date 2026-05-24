#include <RF/receiver.h>

bool readRFSignal(const String& name)
{
    if (!mySwitch.available())
    {
        return false;
    }

    uint32_t code = mySwitch.getReceivedValue();
    uint16_t bits = mySwitch.getReceivedBitlength();
    uint16_t protocol = mySwitch.getReceivedProtocol();
    uint16_t pulse = mySwitch.getReceivedDelay();

    if (code == 0 || signalCount >= MAX_SIGNALS)
    {
        mySwitch.resetAvailable();
        return false;
    }

    signals[signalCount].name = name;
    signals[signalCount].code = code;
    signals[signalCount].bits = bits;
    signals[signalCount].protocol = protocol;
    signals[signalCount].pulse = pulse;
    signalCount++;

    mySwitch.resetAvailable();
    return true;
}
