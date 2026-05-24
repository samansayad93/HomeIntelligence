#include <IR/transmitter.h>

bool transmitIRSignal(const IRSignal* signal)
{
    if (signal == nullptr || signal->rawLength == 0)
    {
        return false;
    }

    irTransmitter.sendRaw(signal->rawData, signal->rawLength, 38);
    return true;
}
