#include <MQTT/mqtt.h>

#include <PubSubClient.h>
#include <WiFi.h>

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);
static MqttCommandHandler mqttCommandHandler = nullptr;
static unsigned long lastReconnectAttempt = 0;

static String topicFor(const String& subTopic)
{
    String topic = MQTT_BASE_TOPIC;
    if (!subTopic.startsWith("/"))
    {
        topic += "/";
    }
    topic += subTopic;
    return topic;
}

static void mqttCallback(char* topic, byte* payload, unsigned int length)
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

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting WiFi to ");
    Serial.println(WIFI_SSID);
}

static bool connectMQTT()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        connectWiFi();
        return false;
    }

    Serial.print("Connecting MQTT to ");
    Serial.print(MQTT_HOST);
    Serial.print(":");
    Serial.println(MQTT_PORT);

    bool connected = false;
    if (strlen(MQTT_USER) > 0)
    {
        connected = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD);
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
    WiFi.setAutoReconnect(true);
    connectWiFi();
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
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

bool publishMQTT(const String& subTopic, const String& payload, bool retained)
{
    if (!mqttClient.connected())
    {
        return false;
    }

    String topic = topicFor(subTopic);
    return mqttClient.publish(topic.c_str(), payload.c_str(), retained);
}
