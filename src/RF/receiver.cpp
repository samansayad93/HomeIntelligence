#include <RF/receiver.h>
#include <Memory/memory.h>

void resetRFReceiver()
{
    rfSwitch433.resetAvailable();
    rfSwitch315.resetAvailable();
}

bool readRFSignal(const String &name)
{
    // Both modules are always listening; whichever band actually receives
    // the signal is the one that fired.
    RCSwitch *activeSwitch = nullptr;
    uint16_t band = 0;

    if (rfSwitch433.available())
    {
        activeSwitch = &rfSwitch433;
        band = 433;
    }
    else if (rfSwitch315.available())
    {
        activeSwitch = &rfSwitch315;
        band = 315;
    }
    else
    {
        return false;
    }

    uint32_t code = activeSwitch->getReceivedValue();
    uint16_t bits = activeSwitch->getReceivedBitlength();
    uint16_t protocol = activeSwitch->getReceivedProtocol();
    uint16_t pulse = activeSwitch->getReceivedDelay();

    if (code == 0 || signalCount >= MAX_SIGNALS)
    {
        activeSwitch->resetAvailable();
        return false;
    }

    signals[signalCount].name = name;
    signals[signalCount].code = code;
    signals[signalCount].bits = bits;
    signals[signalCount].protocol = protocol;
    signals[signalCount].pulse = pulse;
    signals[signalCount].band = band;
    signalCount++;

    activeSwitch->resetAvailable();
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
            Serial.print(receiveName);
            Serial.print(" (");
            Serial.print(signals[signalCount - 1].band);
            Serial.println(" MHz)");
            pulseStatusLED();
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
