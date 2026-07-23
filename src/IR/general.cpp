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

    JsonObject obj;
    String payload;

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

        obj["index"] = i+1;
        obj["name"] = irSignals[i].name;
        obj["protocol"] = static_cast<int16_t>(irSignals[i].protocol);
        obj["value"] = irSignals[i].value;
        obj["bits"] = irSignals[i].bits;
        serializeJsonPretty(obj,payload);
        publishMQTT(MQTT_IR_SIGNAL_TOPIC,payload);
    }
}