#include <config.h>
#include <LDR/ldr.h>
#include <MQ2/mq2.h>
#include <MQTT/mqtt.h>
#include <DHT/dht.h>
#include <PIR/pir.h>
#include <Memory/memory.h>
#include <Provisioning/provisioning.h>
#include <IR/general.h>
#include <IR/receiver.h>
#include <IR/transmitter.h>
#include <RF/general.h>
#include <RF/receiver.h>
#include <RF/transmitter.h>

String receiveName = "";
bool receivingRF = false;
bool receivingIR = false;
unsigned long lastSensorPrint = 0;
unsigned long mq2ReadyAt = 0;
unsigned long nextMQ2ReadAt = 0;

void handleMqttCommand(const String& command);

void printHelp()
{
    Serial.println("Commands:");
    Serial.println("  rf rx <name>  - wait for an RF signal and save it");
    Serial.println("  rf tx <name>  - transmit a saved RF signal");
    Serial.println("  rf list       - list saved RF signals");
    Serial.println("  ir rx <name>  - wait for an IR signal and save it");
    Serial.println("  ir tx <name>  - transmit a saved IR signal");
    Serial.println("  ir list       - list saved IR signals");
    Serial.println("  MQTT command topic: homeinteligence/command");
    Serial.println("  help          - show this help");
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
    }
}

void printMQ2()
{
    unsigned long now = millis();

    if (mq2ReadyAt == 0)
    {
        mq2ReadyAt = now + MQ2_PREHEAT_DURATION;
        nextMQ2ReadAt = mq2ReadyAt;
        Serial.println("MQ2 warming up...");
        return;
    }

    if (now < mq2ReadyAt)
    {
        return;
    }

    if (now < nextMQ2ReadAt)
    {
        return;
    }

    int gas = readMQ2();
    Serial.print("MQ2=");
    Serial.println(gas);
    publishMQTT("sensor/MQ2", String(gas));

    nextMQ2ReadAt += MQ2_READ_INTERVAL;
    if (now > nextMQ2ReadAt)
    {
        nextMQ2ReadAt = now + MQ2_READ_INTERVAL;
    }
}

void handleSerialCommand(const String& command)
{
    int separator = command.indexOf(' ');
    String action = separator == -1 ? command : command.substring(0, separator);
    String argument = separator == -1 ? "" : command.substring(separator + 1);

    action.trim();
    argument.trim();
    action.toLowerCase();

    if (action == "rf" || action == "ir")
    {
        int subSeparator = argument.indexOf(' ');
        String subAction = subSeparator == -1 ? argument : argument.substring(0, subSeparator);
        String name = subSeparator == -1 ? "" : argument.substring(subSeparator + 1);

        subAction.trim();
        name.trim();
        subAction.toLowerCase();

        if (action == "rf" && subAction == "rx")
        {
            if (name.length() == 0)
            {
                Serial.println("Missing signal name. Use: rf rx <name>");
                return;
            }

            if (signalCount >= MAX_SIGNALS)
            {
                Serial.println("RF signal storage is full");
                return;
            }

            if (findRFSignalByName(name) != nullptr)
            {
                Serial.println("IR signal name already exists");
                return;
            }

            receiveName = name;
            receivingRF = true;
            receivingIR = false;
            Serial.print("Waiting for RF signal named: ");
            Serial.println(receiveName);
            return;
        }

        if (action == "rf" && subAction == "tx")
        {
            if (name.length() == 0)
            {
                Serial.println("Missing signal name. Use: rf tx <name>");
                return;
            }

            RFSignal* signal = findRFSignalByName(name);
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

        if (action == "rf" && subAction == "list")
        {
            listRFSignals();
            return;
        }

        if (action == "ir" && subAction == "rx")
        {
            if (name.length() == 0)
            {
                Serial.println("Missing signal name. Use: ir rx <name>");
                return;
            }

            if (irSignalCount >= MAX_SIGNALS)
            {
                Serial.println("IR signal storage is full");
                return;
            }

            if (findIRSignalByName(name) != nullptr)
            {
                Serial.println("IR signal name already exists");
                return;
            }

            receiveName = name;
            receivingIR = true;
            receivingRF = false;
            resetIRReceiver();
            Serial.print("Waiting for IR signal named: ");
            Serial.println(receiveName);
            return;
        }

        if (action == "ir" && subAction == "tx")
        {
            if (name.length() == 0)
            {
                Serial.println("Missing signal name. Use: ir tx <name>");
                return;
            }

            IRSignal* signal = findIRSignalByName(name);
            if (signal == nullptr)
            {
                Serial.println("IR signal not found");
                return;
            }

            if (transmitIRSignal(signal))
            {
                Serial.print("Transmitted IR signal: ");
                Serial.println(name);
            }
            else
            {
                Serial.println("Failed to transmit IR signal");
            }
            return;
        }

        if (action == "ir" && subAction == "list")
        {
            listIRSignals();
            return;
        }

        Serial.println("Unknown command. Type: help");
        return;
    }

    if (action == "help")
    {
        printHelp();
        return;
    }

    Serial.println("Unknown command. Type: help");
}

void handleMqttCommand(const String& command)
{
    handleSerialCommand(command);
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

    if (readRFSignal(receiveName))
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

void handleIRReceive()
{
    if (!receivingIR)
    {
        return;
    }

    if (readIRSignal(receiveName))
    {
        receivingIR = false;
        if (saveIRSignals())
        {
            Serial.print("Saved IR signal: ");
            Serial.println(receiveName);
        }
        else
        {
            Serial.println("Received IR signal, but failed to save it");
        }
    }
}

void printSensors()
{
    if (receivingIR || receivingRF){
        return;
    }

    if (millis() - lastSensorPrint < SENSOR_READ_INTERVAL)
    {
        printMQ2();
        return;
    }

    Serial.println(".............................");
    int LDR = readLDR();
    Serial.print("LDR=");
    Serial.println(LDR);
    publishMQTT("sensor/LDR", String(LDR));
    float temp = readTemperature();
    Serial.print("TEMP=");
    Serial.println(temp);
    publishMQTT("sensor/TEMP", isnan(temp) ? "0" : String(temp, 1));
    float humidity = readHumidity();
    Serial.print("HUM=");
    Serial.println(humidity);
    publishMQTT("sensor/HUM", isnan(humidity) ? "0" : String(humidity, 1));
    int motion = readPIR();
    Serial.print("PIR=");
    Serial.println(motion);
    publishMQTT("sensor/PIR", String(motion));
    Serial.println(".............................");

    lastSensorPrint = millis();
    printMQ2();
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    if (!isProvisioned())
    {
        runProvisioningPortal();
    }

    setupMQ2();
    setupDHT();
    setupPIR();
    setupRF();
    setupIR();
    setupMQTT(handleMqttCommand);
    mq2ReadyAt = millis() + MQ2_PREHEAT_DURATION;
    nextMQ2ReadAt = mq2ReadyAt;
    loadRFSignals();
    loadIRSignals();
    printHelp();
}

void loop()
{
    handleSerial();
    handleMQTT();
    handleRFReceive();
    handleIRReceive();
    printSensors();
}
