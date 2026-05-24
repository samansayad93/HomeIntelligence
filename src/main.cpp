#include <config.h>
#include <LDR/ldr.h>
#include <DHT/dht.h>
#include <PIR/pir.h>
#include <Memory/memory.h>
#include <RF/general.h>
#include <RF/receiver.h>
#include <RF/transmitter.h>

String receiveName = "";
bool receivingRF = false;
unsigned long lastSensorPrint = 0;

void printHelp()
{
    Serial.println("Commands:");
    Serial.println("  rx <name>  - wait for an RF signal and save it with name");
    Serial.println("  tx <name>  - transmit a saved RF signal by name");
    Serial.println("  list       - list saved RF signals");
    Serial.println("  help       - show this help");
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

void handleSerialCommand(const String& command)
{
    int separator = command.indexOf(' ');
    String action = separator == -1 ? command : command.substring(0, separator);
    String name = separator == -1 ? "" : command.substring(separator + 1);

    action.trim();
    name.trim();
    action.toLowerCase();

    if (action == "rx")
    {
        if (name.length() == 0)
        {
            Serial.println("Missing signal name. Use: rx <name>");
            return;
        }

        if (signalCount >= MAX_SIGNALS)
        {
            Serial.println("RF signal storage is full");
            return;
        }

        receiveName = name;
        receivingRF = true;
        Serial.print("Waiting for RF signal named: ");
        Serial.println(receiveName);
        return;
    }

    if (action == "tx")
    {
        if (name.length() == 0)
        {
            Serial.println("Missing signal name. Use: tx <name>");
            return;
        }

        Signal* signal = findRFSignalByName(name);
        if (signal == nullptr)
        {
            Serial.println("RF signal not found");
            return;
        }

        if (transmitRFSignal(signal))
        {
            Serial.print("Transmitted RF signal: ");
            Serial.println(name);
        }
        else
        {
            Serial.println("Failed to transmit RF signal");
        }
        return;
    }

    if (action == "list")
    {
        listRFSignals();
        return;
    }

    if (action == "help")
    {
        printHelp();
        return;
    }

    Serial.println("Unknown command. Type: help");
}

void handleSerial()
{
    if (!Serial.available())
    {
        return;
    }

    String command = Serial.readStringUntil('\n');
    command.trim();

    if (command.length() > 0)
    {
        handleSerialCommand(command);
    }
}

void handleRFReceive()
{
    if (!receivingRF)
    {
        return;
    }

    if (readRFRX(receiveName))
    {
        receivingRF = false;
        if (saveRFSignals())
        {
            Serial.print("Saved RF signal: ");
            Serial.println(receiveName);
        }
        else
        {
            Serial.println("Received RF signal, but failed to save it");
        }
    }
}

void printSensors()
{
    if (millis() - lastSensorPrint < SENSOR_READ_INTERVAL)
    {
        return;
    }

    
    int LDR = readLDR();
    Serial.println(String(LDR));
    float temp = readTemperature();
    Serial.println(String(temp));
    float humidity = readHumidity();
    Serial.println(String(humidity));
    int motion = readPIR();
    Serial.println(String(motion));
    lastSensorPrint = millis();
}

void setup()
{
    Serial.begin(115200);
    delay(200);
    setupDHT();
    setupPIR();
    setupRF();
    loadRFSignals();
    printHelp();
}

void loop()
{
    handleSerial();
    handleRFReceive();
    printSensors();
}
