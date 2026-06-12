#ifndef MQTT_COMMUNICATION_H
#define MQTT_COMMUNICATION_H

#include <config.h>

typedef void (*MqttCommandHandler)(const String& command);

void setupMQTT(MqttCommandHandler commandHandler);
void handleMQTT();
bool isMQTTConnected();
bool publishMQTT(const String& subTopic, const String& payload, bool retained = false);

#endif
