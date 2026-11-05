#pragma once
#include <Arduino.h>
#include "FanController.h"

class FanManager {
public:
    void begin();
    void loop();

    void reportSensor(bool ok, float tempCelsius);
    void setAuto();
    void setManual(uint8_t duty);
    void applySettings();

    uint8_t getDuty() const { return duty; }
    bool isAuto() const { return !controller.isManual(); }
    uint8_t getManualDuty() const { return controller.manualDuty(); }
    bool hasSensorFault() const { return controller.sensorFault(millis()); }
    bool isFailsafeActive() const { return controller.failsafeActive(millis()); }
    bool isQuietActive() const { return controller.quietActive(); }

private:
    FanController controller;
    uint8_t duty = 0;
    unsigned long lastTick = 0;
    unsigned long lastStatePublish = 0;
    uint8_t lastPublishedPercent = 255;
    bool lastFault = false;
    bool stateChanged = true;
    bool settingsChanged = true;

    bool quietHoursNow() const;
    void publishChanges(unsigned long now);
};

extern FanManager fanManager;
