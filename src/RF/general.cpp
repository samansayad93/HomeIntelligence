#include <RF/general.h>

RFSignal signals[MAX_SIGNALS];
size_t signalCount = 0;

RCSwitch mySwitch = RCSwitch();

void setupRF()
{
    mySwitch.enableReceive(RF_RX_PIN);
    mySwitch.enableTransmit(RF_TX_PIN);
}

void listRFSignals()
{
    if (signalCount == 0)
    {
        Serial.println("No RF signals saved");
        return;
    }

    for (size_t i = 0; i < signalCount; i++)
    {
        Serial.print(i + 1);
        Serial.print(". ");
        Serial.print(signals[i].name);
        Serial.print(" code=");
        Serial.print(signals[i].code);
        Serial.print(" bits=");
        Serial.print(signals[i].bits);
        Serial.print(" protocol=");
        Serial.print(signals[i].protocol);
        Serial.print(" pulse=");
        Serial.println(signals[i].pulse);
    }
}