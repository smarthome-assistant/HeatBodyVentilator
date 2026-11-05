#pragma once
#include <Arduino.h>

namespace Config {
    constexpr uint16_t HTTP_PORT = 80;
    constexpr uint8_t LED_PIN    = 27;
    constexpr uint16_t NUM_LEDS  = 1;
    constexpr char FIRMWARE_VERSION[] = "0.0.3";

    constexpr uint8_t FAN_PWM_PIN = 22;
    constexpr uint32_t FAN_PWM_FREQUENCY = 25000;
    constexpr uint8_t FAN_PWM_RESOLUTION = 8;

    constexpr char AP_SSID[] = "Smarthome-Assistant-AP";
    constexpr char AP_PASS[] = "smarthome-assistant.info";

    extern char WEB_PASSWORD[32];
    extern char DEVICE_NAME[32];
    extern char MQTT_SERVER[64];
    extern uint16_t MQTT_PORT;
    extern char MQTT_USER[32];
    extern char MQTT_PASS[32];
    extern char MQTT_TOPIC[64];
    extern char TEMP_UNIT[16];
    extern bool AP_ENABLED;
    extern char LAST_PASSWORD_CHANGE[32];
    
    extern float TEMP_START;
    extern float TEMP_MAX;
    extern bool AUTO_PWM_ENABLED;
    extern uint8_t FAN_MANUAL_DUTY;
    extern uint8_t FAN_MIN_PERCENT;
    extern uint8_t FAN_FAILSAFE_PERCENT;
    extern bool QUIET_ENABLED;
    extern uint16_t QUIET_START_MINUTE;
    extern uint16_t QUIET_END_MINUTE;
    extern uint8_t QUIET_MAX_PERCENT;

    void loadSettings();
    void saveWebPassword(const char* newPassword);
    void saveDeviceName(const char* newName);
    void saveMQTTSettings(const char* server, uint16_t port, const char* user, const char* pass, const char* topic);
    void saveTempUnit(const char* unit);
    void saveAPEnabled(bool enabled);
    void saveTempMapping(float startTemp, float maxTemp);
    void saveAutoPWMEnabled(bool enabled);
    void saveManualDuty(uint8_t duty);
    void saveFanSpeedLimits(uint8_t minPercent, uint8_t failsafePercent);
    void saveQuietHours(bool enabled, uint16_t startMinute, uint16_t endMinute, uint8_t maxPercent);
    void factoryReset();
    String getLastPasswordChange();
}
