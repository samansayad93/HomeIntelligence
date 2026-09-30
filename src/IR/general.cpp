#include <IR/general.h>

IRSignal irSignals[MAX_SIGNALS];
size_t irSignalCount = 0;

IRrecv irReceiver(IR_RX_PIN);
IRsend irTransmitter(IR_TX_PIN);

void setupIR()
{
    irReceiver.enableIRIn();
    irTransmitter.begin();
}

void listIRSignals()
{
    if (irSignalCount == 0)
    {
        Serial.println("No IR signals saved");
        return;
    }

    for (size_t i = 0; i < irSignalCount; i++)
    {
        Serial.print(i + 1);
        Serial.print(". ");
        Serial.print(irSignals[i].name);
        Serial.print(" protocol=");
        Serial.print(typeToString(irSignals[i].protocol));
        Serial.print(" bits=");
        Serial.print(irSignals[i].bits);
        Serial.print(" rawLength=");
        Serial.println(irSignals[i].rawLength);

        JsonDocument doc;
        JsonObject obj = doc.to<JsonObject>();
        obj["index"] = i + 1;
        obj["name"] = irSignals[i].name;
        obj["protocol"] = static_cast<int16_t>(irSignals[i].protocol);
        obj["value"] = irSignals[i].value;
        obj["bits"] = irSignals[i].bits;

        String payload;
        serializeJson(obj, payload);
        publishMQTT(MQTT_IR_SIGNAL_TOPIC, payload);
    }
}

IRSignal *findIRSignalByName(const String &name)
{
    for (size_t i = 0; i < irSignalCount; i++)
    {
        if (irSignals[i].name == name)
        {
            return &irSignals[i];
        }
    }
    return nullptr;
}