#include <RF/general.h>

RFSignal signals[MAX_SIGNALS];
size_t signalCount = 0;

RCSwitch rfSwitch433 = RCSwitch();
RCSwitch rfSwitch315 = RCSwitch();

RCSwitch *getRFSwitch(uint16_t band)
{
    return band == 433 ? &rfSwitch433 : &rfSwitch315;
}

void setupRF()
{
    rfSwitch433.enableReceive(RF_RX_433_PIN);
    rfSwitch433.enableTransmit(RF_TX_433_PIN);
    rfSwitch315.enableReceive(RF_RX_315_PIN);
    rfSwitch315.enableTransmit(RF_TX_315_PIN);
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
        Serial.print(" band=");
        Serial.print(signals[i].band);
        Serial.print(" code=");
        Serial.print(signals[i].code);
        Serial.print(" bits=");
        Serial.print(signals[i].bits);
        Serial.print(" protocol=");
        Serial.print(signals[i].protocol);
        Serial.print(" pulse=");
        Serial.println(signals[i].pulse);

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        obj["index"] = i + 1;
        obj["name"] = signals[i].name;
        obj["code"] = signals[i].code;
        obj["bits"] = signals[i].bits;
        obj["protocol"] = signals[i].protocol;
        obj["pulse"] = signals[i].pulse;
        obj["band"] = signals[i].band;

        String payload;
        serializeJson(obj, payload);
        publishMQTT(MQTT_RF_SIGNAL_TOPIC, payload);
    }
}