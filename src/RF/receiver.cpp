#include <RF/receiver.h>
#include <Memory/memory.h>

void resetRFReceiver()
{
    mySwitch.resetAvailable();
}

bool readRFSignal(const String &name)
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

bool handleRFReceive(bool &receivingRF, const String &receiveName)
{
    if (!receivingRF)
    {
        return false;
    }

    if (readRFSignal(receiveName))
    {
        receivingRF = false;
        if (saveRFSignals())
        {
            Serial.print("Saved RF signal: ");
            Serial.println(receiveName);
            return true;
        }
        else
        {
            if (signalCount > 0)
            {
                signalCount--;
            }
            Serial.println("Received RF signal, but failed to save it");
        }
    }

    return false;
}
