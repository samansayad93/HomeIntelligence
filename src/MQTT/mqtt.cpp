#include <MQTT/mqtt.h>

#include <Provisioning/provisioning.h>
#include <PubSubClient.h>
#include <WiFi.h>

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);
static MqttCommandHandler mqttCommandHandler = nullptr;
static unsigned long lastReconnectAttempt = 0;

static String topicFor(const String &subTopic)
{
    String topic = MQTT_BASE_TOPIC;
    if (!subTopic.startsWith("/"))
    {
        topic += "/";
    }
    topic += subTopic;
    return topic;
}

static void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String command;
    command.reserve(length + 1);

    for (unsigned int i = 0; i < length; i++)
    {
        command += static_cast<char>(payload[i]);
    }

    command.trim();
    Serial.print("MQTT command on ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(command);

    if (command.length() > 0 && mqttCommandHandler != nullptr)
    {
        mqttCommandHandler(command);
    }
}

static void connectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    const char *ssid = provisioningConfig.wifiSSID.length() > 0
                           ? provisioningConfig.wifiSSID.c_str()
                           : WIFI_SSID;
    const char *wpass = provisioningConfig.wifiPassword.length() > 0
                            ? provisioningConfig.wifiPassword.c_str()
                            : WIFI_PASSWORD;

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, wpass);
    Serial.print("Connecting WiFi to ");
    Serial.println(ssid);
}

static bool connectMQTT()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        connectWiFi();
        return false;
    }

    const char *host = provisioningConfig.mqttHost.length() > 0
                           ? provisioningConfig.mqttHost.c_str()
                           : MQTT_HOST;
    uint16_t port = provisioningConfig.mqttPort > 0
                        ? provisioningConfig.mqttPort
                        : MQTT_PORT;
    const char *mqttUser = provisioningConfig.mqttUser.length() > 0
                               ? provisioningConfig.mqttUser.c_str()
                               : MQTT_USER;
    const char *mqttPass = provisioningConfig.mqttPassword.length() > 0
                               ? provisioningConfig.mqttPassword.c_str()
                               : MQTT_PASSWORD;

    Serial.print("Connecting MQTT to ");
    Serial.print(host);
    Serial.print(":");
    Serial.println(port);

    bool connected = false;
    if (strlen(mqttUser) > 0)
    {
        connected = mqttClient.connect(MQTT_CLIENT_ID, mqttUser, mqttPass);
    }
    else
    {
        connected = mqttClient.connect(MQTT_CLIENT_ID);
    }

    if (!connected)
    {
        Serial.print("MQTT connect failed, state=");
        Serial.println(mqttClient.state());
        return false;
    }

    String commandTopic = topicFor("command");
    mqttClient.subscribe(commandTopic.c_str());
    publishMQTT("status", "online", true);
    Serial.print("MQTT subscribed: ");
    Serial.println(commandTopic);
    return true;
}

void setupMQTT(MqttCommandHandler commandHandler)
{
    mqttCommandHandler = commandHandler;

    const char *host = provisioningConfig.mqttHost.length() > 0
                           ? provisioningConfig.mqttHost.c_str()
                           : MQTT_HOST;
    uint16_t port = provisioningConfig.mqttPort > 0
                        ? provisioningConfig.mqttPort
                        : MQTT_PORT;

    WiFi.setAutoReconnect(true);
    connectWiFi();
    mqttClient.setServer(host, port);
    mqttClient.setCallback(mqttCallback);
}

void handleMQTT()
{
    if (WiFi.status() != WL_CONNECTED || !mqttClient.connected())
    {
        unsigned long now = millis();
        if (now - lastReconnectAttempt >= MQTT_RECONNECT_INTERVAL)
        {
            lastReconnectAttempt = now;
            connectMQTT();
        }
        return;
    }

    mqttClient.loop();
}

bool isMQTTConnected()
{
    return mqttClient.connected();
}

bool publishMQTT(const String &subTopic, const String &payload, bool retained)
{
    if (!mqttClient.connected())
    {
        return false;
    }

    String topic = topicFor(subTopic);
    return mqttClient.publish(topic.c_str(), payload.c_str(), retained);
}
