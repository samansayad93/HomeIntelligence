#include <Serial/serial.h>

void printHelp()
{
    Serial.println("Commands:");
    Serial.println("  rf rx <name>  - wait for an RF signal and save it");
    Serial.println("  rf tx <name>  - transmit a saved RF signal");
    Serial.println("  rf list       - list saved RF signals");
    Serial.println("  rf del <name> - delete a saved RF signal");
    Serial.println("  ir rx <name>  - wait for an IR signal and save it");
    Serial.println("  ir tx <name>  - transmit a saved IR signal");
    Serial.println("  ir list       - list saved IR signals");
    Serial.println("  ir del <name> - delete a saved IR signal");
    Serial.println("  buzzer on     - turn on the buzzer");
    Serial.println("  buzzer off    - turn off the buzzer");
    Serial.println("  motion on     - turn on the motion detection");
    Serial.println("  motion off    - turn off the motion detection");
    Serial.println("  help          - show this help");
}

void handleSerialCommand(const String &command)
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
                Serial.println("RF signal name already exists");
                return;
            }

            receiveName = name;
            receivingRF = true;
            receivingIR = false;
            resetRFReceiver();
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

            RFSignal *signal = findRFSignalByName(name);
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

        if (action == "rf" && subAction == "del")
        {
            if (name.length() == 0)
            {
                Serial.println("Missing signal name. Use: rf del <name>");
                return;
            }

            RFSignal *signal = findRFSignalByName(name);
            if (signal == nullptr)
            {
                Serial.println("RF signal not found");
                return;
            }

            size_t index = signal - signals;
            for (size_t i = index; i + 1 < signalCount; i++)
            {
                signals[i] = signals[i + 1];
            }
            signalCount--;

            if (saveRFSignals())
            {
                Serial.print("Deleted RF signal: ");
                Serial.println(name);
            }
            else
            {
                loadRFSignals();
                Serial.println("Failed to delete RF signal");
            }
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

            IRSignal *signal = findIRSignalByName(name);
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

        if (action == "ir" && subAction == "del")
        {
            if (name.length() == 0)
            {
                Serial.println("Missing signal name. Use: ir del <name>");
                return;
            }

            IRSignal *signal = findIRSignalByName(name);
            if (signal == nullptr)
            {
                Serial.println("IR signal not found");
                return;
            }

            size_t index = signal - irSignals;
            for (size_t i = index; i + 1 < irSignalCount; i++)
            {
                irSignals[i] = irSignals[i + 1];
            }
            irSignalCount--;

            if (saveIRSignals())
            {
                Serial.print("Deleted IR signal: ");
                Serial.println(name);
            }
            else
            {
                loadIRSignals();
                Serial.println("Failed to delete IR signal");
            }
            return;
        }

        Serial.println("Unknown command. Type: help");
        return;
    }

    if (action == "buzzer")
    {
        int subSeparator = argument.indexOf(' ');
        String subAction = subSeparator == -1 ? argument : argument.substring(0, subSeparator);
        String name = subSeparator == -1 ? "" : argument.substring(subSeparator + 1);

        if (subAction == "on")
        {
            startAlarm();
            return;
        }

        if (subAction == "off")
        {
            stopAlarm();
            return;
        }

        Serial.println("Unknown command. Type: help");
        return;
    }

    if (action == "motion")
    {
        int subSeparator = argument.indexOf(' ');
        String subAction = subSeparator == -1 ? argument : argument.substring(0, subSeparator);
        String name = subSeparator == -1 ? "" : argument.substring(subSeparator + 1);

        if (subAction == "on")
        {
            turnONMotionDetection();
            return;
        }

        if (subAction == "off")
        {
            turnOFFMotionDetection();
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
