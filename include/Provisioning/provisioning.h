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
void runProvisioningPortal();

#endif
