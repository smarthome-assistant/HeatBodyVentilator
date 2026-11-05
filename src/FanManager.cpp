#include "FanManager.h"
#include "Config.h"
#include "MQTTManager.h"
#include "SettingsRules.h"
#include <time.h>

FanManager fanManager;

namespace {
    const unsigned long TICK_INTERVAL = 100;
    const unsigned long STATE_PUBLISH_INTERVAL = 2000;
    // Anything earlier means NTP has not set the clock yet.
    const time_t EARLIEST_VALID_TIME = 1700000000;
}

void FanManager::begin() {
    if (ledcAttach(Config::FAN_PWM_PIN, Config::FAN_PWM_FREQUENCY, Config::FAN_PWM_RESOLUTION)) {
        ledcWrite(Config::FAN_PWM_PIN, 0);
        Serial.printf("Fan PWM on pin %d at %lu Hz\n", Config::FAN_PWM_PIN, (unsigned long)Config::FAN_PWM_FREQUENCY);
    } else {
        Serial.println("ERROR: Fan PWM attach failed");
    }

    applySettings();
    if (Config::AUTO_PWM_ENABLED) {
        controller.setAuto();
    } else {
        controller.setManual(Config::FAN_MANUAL_DUTY);
    }
}

void FanManager::loop() {
    unsigned long now = millis();
    if (now - lastTick < TICK_INTERVAL) {
        return;
    }
    lastTick = now;

    controller.setQuietActive(quietHoursNow());

    uint8_t next = controller.update(now);
    if (next != duty) {
        ledcWrite(Config::FAN_PWM_PIN, next);
        duty = next;
    }

    publishChanges(now);
}

void FanManager::reportSensor(bool ok, float tempCelsius) {
    controller.reportSensor(ok, tempCelsius, millis());
}

void FanManager::setAuto() {
    controller.setAuto();
    if (!Config::AUTO_PWM_ENABLED) {
        Config::saveAutoPWMEnabled(true);
    }
    stateChanged = true;
}

void FanManager::setManual(uint8_t newDuty) {
    controller.setManual(newDuty);
    if (Config::AUTO_PWM_ENABLED) {
        Config::saveAutoPWMEnabled(false);
    }
    if (Config::FAN_MANUAL_DUTY != newDuty) {
        Config::saveManualDuty(newDuty);
    }
    stateChanged = true;
}

void FanManager::applySettings() {
    FanSettings settings;
    settings.startTemp = Config::TEMP_START;
    settings.maxTemp = Config::TEMP_MAX;
    settings.minPercent = Config::FAN_MIN_PERCENT;
    settings.failsafePercent = Config::FAN_FAILSAFE_PERCENT;
    settings.quietMaxPercent = Config::QUIET_MAX_PERCENT;
    controller.setSettings(settings);
    settingsChanged = true;
}

bool FanManager::quietHoursNow() const {
    if (!Config::QUIET_ENABLED) {
        return false;
    }
    time_t now = time(nullptr);
    if (now < EARLIEST_VALID_TIME) {
        return false;
    }
    struct tm local;
    localtime_r(&now, &local);
    return isInQuietHours(local.tm_hour * 60 + local.tm_min, Config::QUIET_START_MINUTE, Config::QUIET_END_MINUTE);
}

void FanManager::publishChanges(unsigned long now) {
    bool fault = controller.sensorFault(now);
    if (fault != lastFault) {
        lastFault = fault;
        stateChanged = true;
    }

    uint8_t percent = dutyToPercent(duty);
    bool speedChanged = percent != lastPublishedPercent && now - lastStatePublish >= STATE_PUBLISH_INTERVAL;
    if (stateChanged || speedChanged) {
        mqttManager.publishFanState();
        lastPublishedPercent = percent;
        lastStatePublish = now;
        stateChanged = false;
    }

    if (settingsChanged) {
        mqttManager.publishFanSettings();
        settingsChanged = false;
    }
}
