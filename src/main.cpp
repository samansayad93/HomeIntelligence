#include <config.h>
#include <Provisioning/provisioning.h>
#include <Serial/serial.h>
#include <Sensors/sensors.h>

String receiveName = "";
bool receivingRF = false;
bool receivingIR = false;

void handleMqttCommand(const String &command)
{
    handleSerialCommand(command);
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    loadProvisioningConfig();
    startConfigAP();

    setupSensors();
    setupMQTT(handleMqttCommand);
    printHelp();
}

void loop()
{
    handleSerial();
    handleMQTT();
    handleConfigAP();
    handleSensors();
}
