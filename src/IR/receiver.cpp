#include <IR/receiver.h>

void resetIRReceiver()
{
    irReceiver.resume();
}

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
    signal.rawLength = min(static_cast<uint16_t>(results.rawlen), static_cast<uint16_t>(MAX_IR_RAW_LENGTH));

    Serial.println(typeToString(results.decode_type));
    Serial.println(results.value, HEX);
    Serial.println(results.bits);
    Serial.println(results.rawlen);

    for (uint16_t i = 0; i < signal.rawLength; i++)
    {
        signal.rawData[i] = results.rawbuf[i] * kRawTick;
    }

    irSignalCount++;
    irReceiver.resume();
    return true;
}
