#include <Memory/memory.h>

static bool ensureLittleFS()
{
    static bool mounted = false;
    if (mounted)
    {
        return true;
    }

    mounted = LittleFS.begin(true);
    if (!mounted)
    {
        Serial.println("LittleFS mount failed");
    }

    return mounted;
}

bool loadMotionDetection()
{
    if (!LittleFS.begin(false))
    {
        return false;
    }

    if (!LittleFS.exists(SETTINGS_FILE_PATH))
    {
        return false;
    }

    File f = LittleFS.open(SETTINGS_FILE_PATH, "r");
    if (!f)
    {
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err)
    {
        return false;
    }

    return doc["motionDetection"] | false;
}

bool saveMotionDetection(bool enabled)
{
    if (!LittleFS.begin(false))
    {
        return false;
    }

    File f = LittleFS.open(SETTINGS_FILE_PATH, "w");
    if (!f)
    {
        return false;
    }

    JsonDocument doc;
    doc["motionDetection"] = enabled;

    bool ok = serializeJson(doc, f) > 0;
    f.close();
    return ok;
}

bool saveRFSignals()
{
    if (!ensureLittleFS())
    {
        return false;
    }

    File f = LittleFS.open(RF_FILE_PATH, "w");
    if (!f)
    {
        Serial.println("Failed to open RF signal file for writing");
        return false;
    }

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (size_t i = 0; i < signalCount; i++)
    {
        JsonObject obj = arr.add<JsonObject>();
        obj["name"] = signals[i].name;
        obj["code"] = signals[i].code;
        obj["bits"] = signals[i].bits;
        obj["protocol"] = signals[i].protocol;
        obj["pulse"] = signals[i].pulse;
    }

    if (serializeJsonPretty(doc, f) == 0)
    {
        f.close();
        return false;
    }

    f.close();
    return true;
}

bool loadRFSignals()
{
    if (!ensureLittleFS())
    {
        return false;
    }

    if (!LittleFS.exists(RF_FILE_PATH))
    {
        signalCount = 0;
        return true;
    }

    File f = LittleFS.open(RF_FILE_PATH, "r");
    if (!f)
    {
        Serial.println("Failed to open RF signal file for reading");
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err)
    {
        Serial.print("JSON parse failed: ");
        Serial.println(err.c_str());
        signalCount = 0;
        return false;
    }

    if (!doc.is<JsonArray>())
    {
        Serial.println("RF signal file must contain a JSON array");
        signalCount = 0;
        return false;
    }

    JsonArray arr = doc.as<JsonArray>();
    signalCount = 0;

    for (JsonObject obj : arr)
    {
        if (signalCount >= MAX_SIGNALS)
            break;

        signals[signalCount].name = obj["name"] | "";
        signals[signalCount].code = obj["code"] | 0;
        signals[signalCount].bits = obj["bits"] | 0;
        signals[signalCount].protocol = obj["protocol"] | 1;
        signals[signalCount].pulse = obj["pulse"] | 350;
        signalCount++;
    }

    return true;
}

RFSignal *findRFSignalByName(const String &name)
{
    for (size_t i = 0; i < signalCount; i++)
    {
        if (signals[i].name == name)
        {
            return &signals[i];
        }
    }
    return nullptr;
}

bool saveIRSignals()
{
    if (!ensureLittleFS())
    {
        return false;
    }

    File f = LittleFS.open(IR_FILE_PATH, "w");
    if (!f)
    {
        Serial.println("Failed to open IR signal file for writing");
        return false;
    }

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (size_t i = 0; i < irSignalCount; i++)
    {
        JsonObject obj = arr.add<JsonObject>();
        obj["name"] = irSignals[i].name;
        obj["protocol"] = static_cast<int16_t>(irSignals[i].protocol);
        obj["value"] = irSignals[i].value;
        obj["bits"] = irSignals[i].bits;

        JsonArray raw = obj["raw"].to<JsonArray>();
        for (uint16_t j = 0; j < irSignals[i].rawLength; j++)
        {
            raw.add(irSignals[i].rawData[j]);
        }
    }

    if (serializeJsonPretty(doc, f) == 0)
    {
        f.close();
        return false;
    }

    f.close();
    return true;
}

bool loadIRSignals()
{
    if (!ensureLittleFS())
    {
        return false;
    }

    if (!LittleFS.exists(IR_FILE_PATH))
    {
        irSignalCount = 0;
        return true;
    }

    File f = LittleFS.open(IR_FILE_PATH, "r");
    if (!f)
    {
        Serial.println("Failed to open IR signal file for reading");
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err)
    {
        Serial.print("IR JSON parse failed: ");
        Serial.println(err.c_str());
        irSignalCount = 0;
        return false;
    }

    if (!doc.is<JsonArray>())
    {
        Serial.println("IR signal file must contain a JSON array");
        irSignalCount = 0;
        return false;
    }

    JsonArray arr = doc.as<JsonArray>();
    irSignalCount = 0;

    for (JsonObject obj : arr)
    {
        if (irSignalCount >= MAX_SIGNALS)
            break;

        JsonArray raw = obj["raw"].as<JsonArray>();
        if (raw.isNull() || raw.size() == 0)
            continue;

        IRSignal &signal = irSignals[irSignalCount];
        signal.name = obj["name"] | "";
        signal.protocol = static_cast<decode_type_t>(obj["protocol"] | static_cast<int16_t>(decode_type_t::UNKNOWN));
        signal.value = obj["value"] | 0;
        signal.bits = obj["bits"] | 0;
        signal.rawLength = min(static_cast<uint16_t>(raw.size()), static_cast<uint16_t>(MAX_IR_RAW_LENGTH));

        for (uint16_t i = 0; i < signal.rawLength; i++)
        {
            signal.rawData[i] = raw[i] | 0;
        }

        irSignalCount++;
    }

    return true;
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
