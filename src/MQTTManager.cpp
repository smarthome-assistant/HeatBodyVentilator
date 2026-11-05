#include "MQTTManager.h"
#include "LEDManager.h"
#include "FanManager.h"
#include "SettingsRules.h"
#include <WiFi.h>

const String DEVICE_MODEL = "M5Stack Atom";
const String DEVICE_MANUFACTURER = "SmartHome-Assistant.info";

MQTTManager mqttManager;

MQTTManager::MQTTManager()
    : mqttClient(wifiClient), lastReconnectAttempt(0), lastHeartbeat(0), autoDiscoveryPublished(false), ledCallback(nullptr) {
}

void MQTTManager::begin() {
    if (strlen(Config::MQTT_SERVER) == 0) {
        Serial.println("MQTT: No server configured");
        return;
    }

    mqttClient.setServer(Config::MQTT_SERVER, Config::MQTT_PORT);
    mqttClient.setBufferSize(2048);
    mqttClient.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->handleIncomingMessage(topic, payload, length);
    });

    Serial.println("MQTT: Initialized with server " + String(Config::MQTT_SERVER) + ":" + String(Config::MQTT_PORT));
}

void MQTTManager::loop() {
    if (strlen(Config::MQTT_SERVER) == 0 || WiFi.status() != WL_CONNECTED) {
        return;
    }

    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 30000) {
            lastReconnectAttempt = now;
            if (connect()) {
                Serial.println("MQTT: Connected successfully");
                subscribeToCommands();
                publishAutoDiscovery();
                publishDeviceState();
                autoDiscoveryPublished = true;
            }
        }
    } else {
        mqttClient.loop();

        unsigned long now = millis();
        if (now - lastHeartbeat > 30000) {
            lastHeartbeat = now;
            publishDeviceState();
        }
    }
}

bool MQTTManager::isConnected() {
    return mqttClient.connected();
}

void MQTTManager::disconnect() {
    if (mqttClient.connected()) {
        mqttClient.publish(getStatusTopic().c_str(), "offline", true);
        mqttClient.disconnect();
    }
    autoDiscoveryPublished = false;
}

void MQTTManager::setLEDCallback(void (*callback)(bool state)) {
    ledCallback = callback;
}

bool MQTTManager::connect() {
    String clientId = "ESP32-" + getDeviceId();
    String statusTopic = getStatusTopic();

    bool connected;
    if (strlen(Config::MQTT_USER) > 0) {
        connected = mqttClient.connect(clientId.c_str(), Config::MQTT_USER, Config::MQTT_PASS,
                                     statusTopic.c_str(), 1, true, "offline");
    } else {
        connected = mqttClient.connect(clientId.c_str(), statusTopic.c_str(), 1, true, "offline");
    }

    if (connected) {
        mqttClient.publish(statusTopic.c_str(), "online", true);
        Serial.println("MQTT: Connected as " + clientId);
    } else {
        Serial.println("MQTT: Connection failed, rc=" + String(mqttClient.state()));
    }

    return connected;
}

void MQTTManager::publishAutoDiscovery() {
    if (!mqttClient.connected()) return;

    Serial.println("MQTT: Publishing Home Assistant Auto Discovery...");

    publishSwitchDiscovery();
    publishSensorDiscovery();
    publishButtonDiscovery();
    publishFanDiscovery();

    Serial.println("MQTT: Auto Discovery published");
}

// Adds the fields every entity shares: id, name, availability and the device block.
void MQTTManager::fillEntity(JsonDocument& doc, const String& objectId, const String& name) {
    String deviceId = getDeviceId();
    doc["unique_id"] = deviceId + "_" + objectId;
    doc["name"] = String(Config::DEVICE_NAME) + " " + name;
    doc["availability_topic"] = getStatusTopic();

    JsonObject device = doc.createNestedObject("device");
    device["identifiers"][0] = deviceId;
    device["name"] = Config::DEVICE_NAME;
    device["model"] = DEVICE_MODEL;
    device["manufacturer"] = DEVICE_MANUFACTURER;
    device["sw_version"] = Config::FIRMWARE_VERSION;
    device["configuration_url"] = "http://" + WiFi.localIP().toString();
}

void MQTTManager::publishDiscovery(const String& component, const String& objectId, JsonDocument& doc) {
    if (doc.overflowed()) {
        Serial.println("MQTT: WARNING - discovery payload for " + objectId + " is incomplete (JSON buffer too small)");
    }

    String discoveryTopic = getDiscoveryTopic(component, objectId);
    String payload;
    serializeJson(doc, payload);

    bool result = mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
    Serial.println("MQTT: Discovery " + discoveryTopic + ": " + String(result ? "SUCCESS" : "FAILED"));
}

void MQTTManager::publishSwitchDiscovery() {
    String baseTopic = getHomeAssistantBaseTopic();

    DynamicJsonDocument ledSwitch(1536);
    fillEntity(ledSwitch, "led", "LED");
    ledSwitch["state_topic"] = baseTopic + "/led/state";
    ledSwitch["command_topic"] = baseTopic + "/led/set";
    ledSwitch["payload_on"] = "ON";
    ledSwitch["payload_off"] = "OFF";
    ledSwitch["optimistic"] = false;
    ledSwitch["retain"] = true;
    publishDiscovery("switch", "led", ledSwitch);

    DynamicJsonDocument quietSwitch(1536);
    fillEntity(quietSwitch, "quiet_mode", "Nachtmodus");
    quietSwitch["state_topic"] = baseTopic + "/quiet/state";
    quietSwitch["command_topic"] = baseTopic + "/quiet/set";
    quietSwitch["icon"] = "mdi:weather-night";
    quietSwitch["entity_category"] = "config";
    publishDiscovery("switch", "quiet_mode", quietSwitch);
}

void MQTTManager::publishSensorDiscovery() {
    String baseTopic = getHomeAssistantBaseTopic();

    DynamicJsonDocument wifiSensor(1536);
    fillEntity(wifiSensor, "wifi_signal", "WiFi");
    wifiSensor["state_topic"] = baseTopic + "/sensor/wifi_signal";
    wifiSensor["unit_of_measurement"] = "dBm";
    wifiSensor["device_class"] = "signal_strength";
    wifiSensor["state_class"] = "measurement";
    wifiSensor["entity_category"] = "diagnostic";
    publishDiscovery("sensor", "wifi_signal", wifiSensor);

    DynamicJsonDocument uptimeSensor(1536);
    fillEntity(uptimeSensor, "uptime", "Uptime");
    uptimeSensor["state_topic"] = baseTopic + "/sensor/uptime";
    uptimeSensor["unit_of_measurement"] = "s";
    uptimeSensor["device_class"] = "duration";
    uptimeSensor["state_class"] = "total_increasing";
    uptimeSensor["entity_category"] = "diagnostic";
    publishDiscovery("sensor", "uptime", uptimeSensor);

    DynamicJsonDocument fanSensor(1536);
    fillEntity(fanSensor, "fan_speed", "Lüfter");
    fanSensor["state_topic"] = baseTopic + "/sensor/fan_speed";
    fanSensor["unit_of_measurement"] = "%";
    fanSensor["icon"] = "mdi:fan";
    fanSensor["state_class"] = "measurement";
    publishDiscovery("sensor", "fan_speed", fanSensor);

    DynamicJsonDocument tempSensor(1536);
    fillEntity(tempSensor, "temperature", "Temperatur");
    tempSensor["state_topic"] = baseTopic + "/sensor/temperature";
    tempSensor["unit_of_measurement"] = "°C";
    tempSensor["device_class"] = "temperature";
    tempSensor["state_class"] = "measurement";
    publishDiscovery("sensor", "temperature", tempSensor);

    DynamicJsonDocument faultSensor(1536);
    fillEntity(faultSensor, "sensor_fault", "Sensorfehler");
    faultSensor["state_topic"] = baseTopic + "/sensor/sensor_fault";
    faultSensor["device_class"] = "problem";
    faultSensor["entity_category"] = "diagnostic";
    publishDiscovery("binary_sensor", "sensor_fault", faultSensor);
}

void MQTTManager::publishButtonDiscovery() {
    String baseTopic = getHomeAssistantBaseTopic();

    DynamicJsonDocument restartButton(1536);
    fillEntity(restartButton, "restart", "Restart");
    restartButton["command_topic"] = baseTopic + "/button/restart";
    restartButton["device_class"] = "restart";
    restartButton["entity_category"] = "config";
    publishDiscovery("button", "restart", restartButton);
}

void MQTTManager::publishFanDiscovery() {
    String baseTopic = getHomeAssistantBaseTopic();

    DynamicJsonDocument fan(1536);
    fillEntity(fan, "fan", "Lüfter Steuerung");
    fan["state_topic"] = baseTopic + "/fan/state";
    fan["command_topic"] = baseTopic + "/fan/set";
    fan["percentage_state_topic"] = baseTopic + "/fan/percentage";
    fan["percentage_command_topic"] = baseTopic + "/fan/percentage/set";
    fan["preset_mode_state_topic"] = baseTopic + "/fan/preset";
    fan["preset_mode_command_topic"] = baseTopic + "/fan/preset/set";
    fan["preset_modes"][0] = "auto";
    publishDiscovery("fan", "fan", fan);

    publishNumberDiscovery("start_temp", "Starttemperatur", 0, (int)TEMP_MAPPING_LIMIT_C, "°C");
    publishNumberDiscovery("max_temp", "Maximaltemperatur", 0, (int)TEMP_MAPPING_LIMIT_C, "°C");
    publishNumberDiscovery("min_speed", "Mindestgeschwindigkeit", 1, 100, "%");
    publishNumberDiscovery("quiet_max", "Nachtmodus Maximum", 0, 100, "%");
}

void MQTTManager::publishNumberDiscovery(const String& objectId, const String& name, int min, int max, const String& unit) {
    String baseTopic = getHomeAssistantBaseTopic();

    DynamicJsonDocument number(1536);
    fillEntity(number, objectId, name);
    number["state_topic"] = baseTopic + "/number/" + objectId;
    number["command_topic"] = baseTopic + "/number/" + objectId + "/set";
    number["min"] = min;
    number["max"] = max;
    number["step"] = 1;
    number["mode"] = "box";
    number["unit_of_measurement"] = unit;
    number["entity_category"] = "config";
    publishDiscovery("number", objectId, number);
}

void MQTTManager::subscribeToCommands() {
    String baseTopic = getHomeAssistantBaseTopic();

    const char* commandTopics[] = {
        "/led/set", "/button/restart", "/quiet/set",
        "/fan/set", "/fan/percentage/set", "/fan/preset/set",
        "/number/start_temp/set", "/number/max_temp/set",
        "/number/min_speed/set", "/number/quiet_max/set"
    };
    for (const char* topic : commandTopics) {
        mqttClient.subscribe((baseTopic + topic).c_str());
    }

    Serial.println("MQTT: Subscribed to command topics");
}

void MQTTManager::publishDeviceState() {
    if (!mqttClient.connected()) return;

    String baseTopic = getHomeAssistantBaseTopic();

    int rssi = WiFi.RSSI();
    String wifiTopic = baseTopic + "/sensor/wifi_signal";
    mqttClient.publish(wifiTopic.c_str(), String(rssi).c_str());

    unsigned long uptime = millis() / 1000;
    String uptimeTopic = baseTopic + "/sensor/uptime";
    mqttClient.publish(uptimeTopic.c_str(), String(uptime).c_str());

    publishFanState();
    publishFanSettings();

    mqttClient.publish(getStatusTopic().c_str(), "online", true);
}

void MQTTManager::publishFanSpeed(int pwmDuty) {
    if (!mqttClient.connected()) return;

    String baseTopic = getHomeAssistantBaseTopic();
    String fanTopic = baseTopic + "/sensor/fan_speed";

    float percentage = (pwmDuty / 255.0) * 100.0;

    mqttClient.publish(fanTopic.c_str(), String(percentage, 1).c_str());
    Serial.printf("MQTT: Published fan speed: %.1f%%\n", percentage);
}

void MQTTManager::publishFanState() {
    if (!mqttClient.connected()) return;

    String baseTopic = getHomeAssistantBaseTopic();
    uint8_t duty = fanManager.getDuty();

    mqttClient.publish((baseTopic + "/fan/state").c_str(), duty > 0 ? "ON" : "OFF");
    mqttClient.publish((baseTopic + "/fan/percentage").c_str(), String(dutyToPercent(duty)).c_str());
    // "None" tells Home Assistant that no preset is active, i.e. manual mode.
    mqttClient.publish((baseTopic + "/fan/preset").c_str(), fanManager.isAuto() ? "auto" : "None");
    mqttClient.publish((baseTopic + "/sensor/sensor_fault").c_str(), fanManager.hasSensorFault() ? "ON" : "OFF");
    publishFanSpeed(duty);
}

void MQTTManager::publishFanSettings() {
    if (!mqttClient.connected()) return;

    String baseTopic = getHomeAssistantBaseTopic();
    mqttClient.publish((baseTopic + "/number/start_temp").c_str(), String(Config::TEMP_START, 1).c_str());
    mqttClient.publish((baseTopic + "/number/max_temp").c_str(), String(Config::TEMP_MAX, 1).c_str());
    mqttClient.publish((baseTopic + "/number/min_speed").c_str(), String(Config::FAN_MIN_PERCENT).c_str());
    mqttClient.publish((baseTopic + "/number/quiet_max").c_str(), String(Config::QUIET_MAX_PERCENT).c_str());
    mqttClient.publish((baseTopic + "/quiet/state").c_str(), Config::QUIET_ENABLED ? "ON" : "OFF");
}

void MQTTManager::publishTemperature(float tempCelsius) {
    if (!mqttClient.connected()) return;

    String baseTopic = getHomeAssistantBaseTopic();
    String tempTopic = baseTopic + "/sensor/temperature";

    mqttClient.publish(tempTopic.c_str(), String(tempCelsius, 1).c_str());
    Serial.printf("MQTT: Published temperature: %.1f°C\n", tempCelsius);
}

void MQTTManager::publishSensorData(const String& sensor, float value, const String& unit) {
    if (!mqttClient.connected()) return;

    String topic = getBaseTopic() + "/sensor/" + sensor;
    mqttClient.publish(topic.c_str(), String(value).c_str());
}

void MQTTManager::handleIncomingMessage(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    String topicStr = String(topic);
    String baseTopic = getHomeAssistantBaseTopic();

    Serial.println("MQTT: Received message on " + topicStr + ": " + message);

    if (topicStr == baseTopic + "/led/set") {
        if (message == "ON") {
            if (ledCallback) {
                ledCallback(true);
            }
            mqttClient.publish((baseTopic + "/led/state").c_str(), "ON", true);
        } else if (message == "OFF") {
            if (ledCallback) {
                ledCallback(false);
            }
            mqttClient.publish((baseTopic + "/led/state").c_str(), "OFF", true);
        }
    }

    if (topicStr == baseTopic + "/button/restart") {
        Serial.println("MQTT: Restart requested via Home Assistant");
        delay(1000);
        ESP.restart();
    }

    handleFanCommand(topicStr.substring(baseTopic.length()), message);
}

// Fan commands from Home Assistant. FanManager publishes the resulting state on its next tick.
void MQTTManager::handleFanCommand(const String& topic, const String& message) {
    if (topic == "/fan/set") {
        if (message == "OFF") {
            fanManager.setManual(0);
        } else if (message == "ON" && !fanManager.isAuto() && fanManager.getManualDuty() == 0) {
            fanManager.setAuto();
        }
    } else if (topic == "/fan/percentage/set") {
        fanManager.setManual(percentToDuty(message.toInt()));
    } else if (topic == "/fan/preset/set") {
        if (message == "auto") {
            fanManager.setAuto();
        }
    } else if (topic == "/quiet/set") {
        Config::saveQuietHours(message == "ON", Config::QUIET_START_MINUTE, Config::QUIET_END_MINUTE, Config::QUIET_MAX_PERCENT);
        fanManager.applySettings();
    } else if (topic == "/number/start_temp/set" || topic == "/number/max_temp/set") {
        float startTemp = topic == "/number/start_temp/set" ? message.toFloat() : Config::TEMP_START;
        float maxTemp = topic == "/number/max_temp/set" ? message.toFloat() : Config::TEMP_MAX;
        if (isValidTempMapping(startTemp, maxTemp)) {
            Config::saveTempMapping(startTemp, maxTemp);
        } else {
            Serial.println("MQTT: Rejected temperature mapping - start must be below max (0-120 °C)");
        }
        // Republishes the stored values, which also resets a rejected value in Home Assistant.
        fanManager.applySettings();
    } else if (topic == "/number/min_speed/set") {
        int percent = message.toInt();
        if (percent >= 1 && percent <= 100) {
            Config::saveFanSpeedLimits(percent, Config::FAN_FAILSAFE_PERCENT);
        }
        fanManager.applySettings();
    } else if (topic == "/number/quiet_max/set") {
        int percent = message.toInt();
        if (percent >= 0 && percent <= 100) {
            Config::saveQuietHours(Config::QUIET_ENABLED, Config::QUIET_START_MINUTE, Config::QUIET_END_MINUTE, percent);
        }
        fanManager.applySettings();
    }
}

String MQTTManager::getDeviceId() {
    String mac = getMacAddress().substring(9);
    mac.replace(":", "_");
    return mac;
}

String MQTTManager::getMacAddress() {
    return WiFi.macAddress();
}

String MQTTManager::getBaseTopic() {
    if (strlen(Config::MQTT_TOPIC) > 0) {
        return String(Config::MQTT_TOPIC) + "/" + getDeviceId();
    }
    return "homeassistant/esp32/" + getDeviceId();
}

String MQTTManager::getHomeAssistantBaseTopic() {
    return "homeassistant/esp32/" + getDeviceId();
}

// The last will, the heartbeat and the discovery availability_topic must all use this one topic.
String MQTTManager::getStatusTopic() {
    return getHomeAssistantBaseTopic() + "/status";
}

String MQTTManager::getDiscoveryTopic(const String& component, const String& objectId) {
    String nodeId = getDeviceId();
    return "homeassistant/" + component + "/" + nodeId + "/" + objectId + "/config";
}
