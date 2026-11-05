#include <Preferences.h>
#include "Config.h"
#include <time.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef SECRET_WEB_PASSWORD
#define SECRET_WEB_PASSWORD "smarthome-assistant.info"
#endif

namespace Config {
    char WEB_PASSWORD[32] = SECRET_WEB_PASSWORD;
    char DEVICE_NAME[32] = "ESP32-Test";
    char MQTT_SERVER[64] = "";
    uint16_t MQTT_PORT = 1883;
    char MQTT_USER[32] = "";
    char MQTT_PASS[32] = "";
    char MQTT_TOPIC[64] = "home/esp32";
    char TEMP_UNIT[16] = "celsius";
    bool AP_ENABLED = true;
    char LAST_PASSWORD_CHANGE[32] = "Nie geändert";
    
    float TEMP_START = 30.0;
    float TEMP_MAX = 80.0;
    bool AUTO_PWM_ENABLED = true;
    uint8_t FAN_MANUAL_DUTY = 0;
    uint8_t FAN_MIN_PERCENT = 10;
    uint8_t FAN_FAILSAFE_PERCENT = 30;
    bool QUIET_ENABLED = false;
    uint16_t QUIET_START_MINUTE = 22 * 60;
    uint16_t QUIET_END_MINUTE = 6 * 60;
    uint8_t QUIET_MAX_PERCENT = 30;

    void loadSettings() {
        Preferences prefs;
        if (!prefs.begin("settings", true)) {
            Serial.println("Warning: Failed to open settings preferences, using defaults");
            return;
        }
        
        String pw = prefs.getString("webpw", SECRET_WEB_PASSWORD);
        strncpy(WEB_PASSWORD, pw.c_str(), sizeof(WEB_PASSWORD));
        WEB_PASSWORD[sizeof(WEB_PASSWORD)-1] = '\0';
        
        String deviceName = prefs.getString("device_name", "ESP32-Test");
        strncpy(DEVICE_NAME, deviceName.c_str(), sizeof(DEVICE_NAME));
        DEVICE_NAME[sizeof(DEVICE_NAME)-1] = '\0';
        
        String mqttServer = prefs.getString("mqtt_server", "");
        strncpy(MQTT_SERVER, mqttServer.c_str(), sizeof(MQTT_SERVER));
        MQTT_SERVER[sizeof(MQTT_SERVER)-1] = '\0';
        
        MQTT_PORT = prefs.getUShort("mqtt_port", 1883);
        
        String mqttUser = prefs.getString("mqtt_user", "");
        strncpy(MQTT_USER, mqttUser.c_str(), sizeof(MQTT_USER));
        MQTT_USER[sizeof(MQTT_USER)-1] = '\0';
        
        String mqttPass = prefs.getString("mqtt_pass", "");
        strncpy(MQTT_PASS, mqttPass.c_str(), sizeof(MQTT_PASS));
        MQTT_PASS[sizeof(MQTT_PASS)-1] = '\0';
        
        String mqttTopic = prefs.getString("mqtt_topic", "home/esp32");
        strncpy(MQTT_TOPIC, mqttTopic.c_str(), sizeof(MQTT_TOPIC));
        MQTT_TOPIC[sizeof(MQTT_TOPIC)-1] = '\0';
        
        String tempUnit = prefs.getString("temp_unit", "celsius");
        strncpy(TEMP_UNIT, tempUnit.c_str(), sizeof(TEMP_UNIT));
        TEMP_UNIT[sizeof(TEMP_UNIT)-1] = '\0';
        
        AP_ENABLED = prefs.getBool("ap_enabled", true);
        
        String lastPwChange = prefs.getString("last_pw_change", "Nie geändert");
        strncpy(LAST_PASSWORD_CHANGE, lastPwChange.c_str(), sizeof(LAST_PASSWORD_CHANGE));
        LAST_PASSWORD_CHANGE[sizeof(LAST_PASSWORD_CHANGE)-1] = '\0';
        
        TEMP_START = prefs.getFloat("temp_start", 30.0);
        TEMP_MAX = prefs.getFloat("temp_max", 80.0);
        AUTO_PWM_ENABLED = prefs.getBool("auto_pwm", true);
        FAN_MANUAL_DUTY = prefs.getUChar("fan_manual", 0);
        FAN_MIN_PERCENT = prefs.getUChar("fan_min_pct", 10);
        FAN_FAILSAFE_PERCENT = prefs.getUChar("fan_fs_pct", 30);
        QUIET_ENABLED = prefs.getBool("quiet_on", false);
        QUIET_START_MINUTE = prefs.getUShort("quiet_start", 22 * 60);
        QUIET_END_MINUTE = prefs.getUShort("quiet_end", 6 * 60);
        QUIET_MAX_PERCENT = prefs.getUChar("quiet_max", 30);

        prefs.end();
        Serial.println("Settings loaded successfully");
    }

    void saveWebPassword(const char* newPassword) {
        Preferences prefs;
        if (!prefs.begin("settings", false)) {
            Serial.println("Error: Failed to open settings preferences for writing");
            return;
        }
        prefs.putString("webpw", newPassword);
        strncpy(WEB_PASSWORD, newPassword, sizeof(WEB_PASSWORD));
        WEB_PASSWORD[sizeof(WEB_PASSWORD)-1] = '\0';
        
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M", timeinfo);
        prefs.putString("last_pw_change", timestamp);
        strncpy(LAST_PASSWORD_CHANGE, timestamp, sizeof(LAST_PASSWORD_CHANGE));
        LAST_PASSWORD_CHANGE[sizeof(LAST_PASSWORD_CHANGE)-1] = '\0';
        
        prefs.end();
    }

    void saveDeviceName(const char* newName) {
        Preferences prefs;
        if (!prefs.begin("settings", false)) {
            Serial.println("Error: Failed to open settings preferences for writing");
            return;
        }
        prefs.putString("device_name", newName);
        strncpy(DEVICE_NAME, newName, sizeof(DEVICE_NAME));
        DEVICE_NAME[sizeof(DEVICE_NAME)-1] = '\0';
        prefs.end();
    }

    void saveMQTTSettings(const char* server, uint16_t port, const char* user, const char* pass, const char* topic) {
        Preferences prefs;
        if (!prefs.begin("settings", false)) {
            Serial.println("Error: Failed to open settings preferences for writing");
            return;
        }
        prefs.putString("mqtt_server", server);
        prefs.putUShort("mqtt_port", port);
        prefs.putString("mqtt_user", user);
        prefs.putString("mqtt_pass", pass);
        prefs.putString("mqtt_topic", topic);
        
        strncpy(MQTT_SERVER, server, sizeof(MQTT_SERVER));
        MQTT_SERVER[sizeof(MQTT_SERVER)-1] = '\0';
        MQTT_PORT = port;
        strncpy(MQTT_USER, user, sizeof(MQTT_USER));
        MQTT_USER[sizeof(MQTT_USER)-1] = '\0';
        strncpy(MQTT_PASS, pass, sizeof(MQTT_PASS));
        MQTT_PASS[sizeof(MQTT_PASS)-1] = '\0';
        strncpy(MQTT_TOPIC, topic, sizeof(MQTT_TOPIC));
        MQTT_TOPIC[sizeof(MQTT_TOPIC)-1] = '\0';
        
        prefs.end();
    }

    void saveTempUnit(const char* unit) {
        Preferences prefs;
        if (!prefs.begin("settings", false)) {
            Serial.println("Error: Failed to open settings preferences for writing");
            return;
        }
        prefs.putString("temp_unit", unit);
        strncpy(TEMP_UNIT, unit, sizeof(TEMP_UNIT));
        TEMP_UNIT[sizeof(TEMP_UNIT)-1] = '\0';
        prefs.end();
    }

    void saveAPEnabled(bool enabled) {
        Preferences prefs;
        if (!prefs.begin("settings", false)) {
            Serial.println("Error: Failed to open settings preferences for writing");
            return;
        }
        prefs.putBool("ap_enabled", enabled);
        AP_ENABLED = enabled;
        prefs.end();
    }

    void factoryReset() {
        Serial.println("Performing factory reset...");
        
        Preferences prefs;
        if (prefs.begin("settings", false)) {
            prefs.clear();
            prefs.end();
            Serial.println("Settings preferences cleared");
        } else {
            Serial.println("Warning: Failed to clear settings preferences");
        }
        
        if (prefs.begin("wifi", false)) {
            prefs.clear();
            prefs.end();
            Serial.println("WiFi preferences cleared");
        } else {
            Serial.println("Warning: Failed to clear WiFi preferences");
        }
        
        strcpy(WEB_PASSWORD, SECRET_WEB_PASSWORD);
        strcpy(DEVICE_NAME, "ESP32-Test");
        strcpy(MQTT_SERVER, "");
        MQTT_PORT = 1883;
        strcpy(MQTT_USER, "");
        strcpy(MQTT_PASS, "");
        strcpy(MQTT_TOPIC, "home/esp32");
        strcpy(TEMP_UNIT, "celsius");
        AP_ENABLED = true;
        strcpy(LAST_PASSWORD_CHANGE, "Nie geändert");
        
        TEMP_START = 30.0;
        TEMP_MAX = 80.0;
        AUTO_PWM_ENABLED = true;
        FAN_MANUAL_DUTY = 0;
        FAN_MIN_PERCENT = 10;
        FAN_FAILSAFE_PERCENT = 30;
        QUIET_ENABLED = false;
        QUIET_START_MINUTE = 22 * 60;
        QUIET_END_MINUTE = 6 * 60;
        QUIET_MAX_PERCENT = 30;

        Serial.println("Factory reset completed");
    }

    void saveTempMapping(float startTemp, float maxTemp) {
        Preferences prefs;
        if (prefs.begin("settings", false)) {
            prefs.putFloat("temp_start", startTemp);
            prefs.putFloat("temp_max", maxTemp);
            prefs.end();
            
            TEMP_START = startTemp;
            TEMP_MAX = maxTemp;
            
            Serial.printf("Temperature mapping saved: Start=%.1f°C, Max=%.1f°C\n", startTemp, maxTemp);
        } else {
            Serial.println("Error: Failed to save temperature mapping");
        }
    }

    void saveAutoPWMEnabled(bool enabled) {
        Preferences prefs;
        if (prefs.begin("settings", false)) {
            prefs.putBool("auto_pwm", enabled);
            prefs.end();
            
            AUTO_PWM_ENABLED = enabled;
            
            Serial.printf("Auto PWM setting saved: %s\n", enabled ? "Enabled" : "Disabled");
        } else {
            Serial.println("Error: Failed to save auto PWM setting");
        }
    }

    void saveManualDuty(uint8_t duty) {
        Preferences prefs;
        if (prefs.begin("settings", false)) {
            prefs.putUChar("fan_manual", duty);
            prefs.end();

            FAN_MANUAL_DUTY = duty;
        } else {
            Serial.println("Error: Failed to save manual fan speed");
        }
    }

    void saveFanSpeedLimits(uint8_t minPercent, uint8_t failsafePercent) {
        Preferences prefs;
        if (prefs.begin("settings", false)) {
            prefs.putUChar("fan_min_pct", minPercent);
            prefs.putUChar("fan_fs_pct", failsafePercent);
            prefs.end();

            FAN_MIN_PERCENT = minPercent;
            FAN_FAILSAFE_PERCENT = failsafePercent;
        } else {
            Serial.println("Error: Failed to save fan speed limits");
        }
    }

    void saveQuietHours(bool enabled, uint16_t startMinute, uint16_t endMinute, uint8_t maxPercent) {
        Preferences prefs;
        if (prefs.begin("settings", false)) {
            prefs.putBool("quiet_on", enabled);
            prefs.putUShort("quiet_start", startMinute);
            prefs.putUShort("quiet_end", endMinute);
            prefs.putUChar("quiet_max", maxPercent);
            prefs.end();

            QUIET_ENABLED = enabled;
            QUIET_START_MINUTE = startMinute;
            QUIET_END_MINUTE = endMinute;
            QUIET_MAX_PERCENT = maxPercent;
        } else {
            Serial.println("Error: Failed to save quiet hours");
        }
    }

    String getLastPasswordChange() {
        return String(LAST_PASSWORD_CHANGE);
    }
}