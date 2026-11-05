#pragma once
#include <stdint.h>

struct FanSettings {
    float startTemp = 30.0f;
    float maxTemp = 80.0f;
    uint8_t minPercent = 10;
    uint8_t failsafePercent = 30;
    uint8_t quietMaxPercent = 30;
};

// Decides the fan's PWM duty (0-255). Has no hardware access so it can be tested on the host.
class FanController {
public:
    explicit FanController(const FanSettings& settings = FanSettings()) : settings_(settings) {}

    void setSettings(const FanSettings& settings) { settings_ = settings; }
    void setAuto() { manual_ = false; }
    void setManual(uint8_t duty) {
        manual_ = true;
        manualDuty_ = duty;
    }
    void setQuietActive(bool active) { quietActive_ = active; }
    void reportSensor(bool ok, float temperature, uint32_t now);

    // Call regularly; returns the duty to write to the PWM pin.
    uint8_t update(uint32_t now);

    bool isManual() const { return manual_; }
    uint8_t manualDuty() const { return manualDuty_; }
    bool sensorFault(uint32_t now) const;
    bool failsafeActive(uint32_t now) const { return !manual_ && sensorFault(now); }
    bool quietActive() const { return quietActive_; }

private:
    float curveDuty();
    float targetDuty(uint32_t now);
    float kickDuty() const;

    FanSettings settings_;
    bool manual_ = false;
    uint8_t manualDuty_ = 0;
    bool quietActive_ = false;

    bool sensorOk_ = false;
    uint32_t sensorBadSince_ = 0;
    float temperature_ = 0.0f;
    bool curveRunning_ = false;
    float lastCurveDuty_ = 0.0f;

    float current_ = 0.0f;
    bool kicking_ = false;
    uint32_t kickStart_ = 0;
    uint32_t lastUpdate_ = 0;
};
