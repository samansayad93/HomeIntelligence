#include <IR/receiver.h>

bool readIRSignal(const String& name)
{
    if (irSignalCount >= MAX_SIGNALS)
    {
        return false;
    }

    decode_results results;
    if (!irReceiver.decode(&results))
    {
        return false;
    }

    if (results.overflow || results.rawlen <= 1)
    {
        irReceiver.resume();
        return false;
    }

    IRSignal& signal = irSignals[irSignalCount];
    signal.name = name;
    signal.protocol = results.decode_type;
    signal.value = results.value;
    signal.bits = results.bits;
    signal.rawLength = min(static_cast<uint16_t>(results.rawlen - 1), static_cast<uint16_t>(MAX_IR_RAW_LENGTH));

    for (uint16_t i = 0; i < signal.rawLength; i++)
    {
        signal.rawData[i] = results.rawbuf[i + 1] * kRawTick;
    }

    irSignalCount++;
    irReceiver.resume();
    return true;
}
