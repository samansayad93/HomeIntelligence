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

bool saveRFSignals()
{
    if (!ensureLittleFS())
    {
        return false;
    }

    File f = LittleFS.open(FILE_PATH, "w");
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

    if (!LittleFS.exists(FILE_PATH))
    {
        signalCount = 0;
        return true;
    }

    File f = LittleFS.open(FILE_PATH, "r");
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

Signal* findRFSignalByName(const String& name){
    for (size_t i=0;i<signalCount;i++){
        if (signals[i].name == name){
            return &signals[i];
        }
    }
    return nullptr;
}
