#include <RF/transmitter.h>

bool transmitRFSignal(const RFSignal *signal)
{
    if (signal == nullptr || signal->code == 0 || signal->bits == 0)
    {
        return false;
    }

    mySwitch.setProtocol(signal->protocol);
    mySwitch.setPulseLength(signal->pulse);
    mySwitch.send(signal->code, signal->bits);
    return true;
}
