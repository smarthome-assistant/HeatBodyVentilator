#pragma once
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Config.h"

class MQTTManager {
public:
    MQTTManager();
    void begin();
    void loop();
    bool isConnected();
    void disconnect();
    
    void publishAutoDiscovery();
    void publishDeviceState();
    
    void publishFanSpeed(int pwmDuty);
    void publishFanState();
    void publishFanSettings();
    void publishTemperature(float tempCelsius);
    
    void publishSensorData(const String& sensor, float value, const String& unit = "");
    void handleIncomingMessage(char* topic, byte* payload, unsigned int length);
    void setLEDCallback(void (*callback)(bool state));
    
private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    unsigned long lastReconnectAttempt;
    unsigned long lastHeartbeat;
    bool autoDiscoveryPublished;
    void (*ledCallback)(bool state);
    
    bool connect();
    void fillEntity(JsonDocument& doc, const String& objectId, const String& name);
    void publishDiscovery(const String& component, const String& objectId, JsonDocument& doc);
    void publishSwitchDiscovery();
    void publishSensorDiscovery();
    void publishButtonDiscovery();
    void publishFanDiscovery();
    void publishNumberDiscovery(const String& objectId, const String& name, int min, int max, const String& unit);
    void subscribeToCommands();
    void handleFanCommand(const String& topic, const String& message);

    String getDeviceId();
    String getMacAddress();
    String getBaseTopic();
    String getStatusTopic();
    String getDiscoveryTopic(const String& component, const String& objectId);
    String getHomeAssistantBaseTopic();
};

extern MQTTManager mqttManager;
