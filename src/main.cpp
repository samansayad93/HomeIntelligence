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

    // Load any saved profile, then bring up the always-on config AP. The board
    // runs in WIFI_AP_STA: it joins the provisioned router as a station (for
    // MQTT) AND keeps broadcasting the `Intelligence-Setup` AP so you can always
    // connect to that SSID and open http://192.168.4.1 to change the config —
    // even while it is correctly connected to the router.
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
