#include <IR/receiver.h>
#include <Memory/memory.h>

void resetIRReceiver()
{
    irReceiver.resume();
}

bool readIRSignal(const String &name)
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

    IRSignal &signal = irSignals[irSignalCount];
    signal.name = name;
    signal.protocol = results.decode_type;
    signal.value = results.value;
    signal.bits = results.bits;
    signal.rawLength = min(static_cast<uint16_t>(results.rawlen - 1), static_cast<uint16_t>(MAX_IR_RAW_LENGTH));

    Serial.println(typeToString(results.decode_type));
    Serial.println(results.value, HEX);
    Serial.println(results.bits);
    Serial.println(results.rawlen);

    for (uint16_t i = 0; i < signal.rawLength; i++)
    {
        signal.rawData[i] = results.rawbuf[i + 1] * kRawTick;
    }

    irSignalCount++;
    irReceiver.resume();
    return true;
}

bool handleIRReceive(bool &receivingIR, const String &receiveName)
{
    if (!receivingIR)
    {
        return false;
    }

    if (readIRSignal(receiveName))
    {
        receivingIR = false;
        if (saveIRSignals())
        {
            Serial.print("Saved IR signal: ");
            Serial.println(receiveName);
            return true;
        }
        else
        {
            if (irSignalCount > 0)
            {
                irSignalCount--;
            }
            Serial.println("Received IR signal, but failed to save it");
        }
    }

    return false;
}
