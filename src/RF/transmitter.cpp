#include <RF/transmitter.h>

bool transmitRFSignal(const RFSignal *signal)
{
    if (signal == nullptr || signal->code == 0 || signal->bits == 0)
    {
        return false;
    }

    RCSwitch *sw = getRFSwitch(signal->band);
    sw->setProtocol(signal->protocol);
    sw->setPulseLength(signal->pulse);
    sw->send(signal->code, signal->bits);
    return true;
}
