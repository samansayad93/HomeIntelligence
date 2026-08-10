#ifndef PROVISIONING_H
#define PROVISIONING_H

#include <Arduino.h>

struct ProvisioningConfig
{
    String wifiSSID;
    String wifiPassword;
    String mqttHost;
    uint16_t mqttPort;
    String mqttUser;
    String mqttPassword;
    String client;
};

extern ProvisioningConfig provisioningConfig;

bool loadProvisioningConfig();
bool saveProvisioningConfig(const ProvisioningConfig &cfg);
bool isProvisioned();

// Always-on soft-AP configuration portal. The board runs in WIFI_AP_STA: it
// joins the provisioned router as a station (for MQTT) AND continuously
// broadcasts the `Intelligence-Setup` AP, so you can always connect to that SSID
// and open http://192.168.4.1 to change the Wi-Fi AP / broker — even while the
// board is correctly connected to the router. Non-blocking: call startConfigAP()
// once in setup(), then handleConfigAP() in loop().
void startConfigAP();
void handleConfigAP();

#endif
