#include "FanController.h"

#include "SettingsRules.h"

namespace {
const float HYSTERESIS_C = 2.0f;
const uint32_t KICK_MS = 1000;
const float RAMP_DUTY_PER_SECOND = 25.0f;
const uint32_t SENSOR_GRACE_MS = 30000;
const float FULL_DUTY = 255.0f;

float lowerOf(float a, float b) { return a < b ? a : b; }
float higherOf(float a, float b) { return a > b ? a : b; }
}  // namespace

void FanController::reportSensor(bool ok, float temperature, uint32_t now) {
    if (ok) {
        sensorOk_ = true;
        temperature_ = temperature;
    } else if (sensorOk_) {
        sensorOk_ = false;
        sensorBadSince_ = now;
    }
}

bool FanController::sensorFault(uint32_t now) const {
    return !sensorOk_ && now - sensorBadSince_ >= SENSOR_GRACE_MS;
}

float FanController::curveDuty() {
    if (curveRunning_) {
        if (temperature_ <= settings_.startTemp - HYSTERESIS_C) {
            curveRunning_ = false;
        }
    } else if (temperature_ > settings_.startTemp) {
        curveRunning_ = true;
    }
    if (!curveRunning_) {
        return 0.0f;
    }

    const float minDuty = percentToDuty(settings_.minPercent);
    if (temperature_ >= settings_.maxTemp) {
        return FULL_DUTY;
    }
    if (temperature_ <= settings_.startTemp) {
        return minDuty;
    }
    const float position = (temperature_ - settings_.startTemp) / (settings_.maxTemp - settings_.startTemp);
    return minDuty + position * (FULL_DUTY - minDuty);
}

float FanController::targetDuty(uint32_t now) {
    if (manual_) {
        if (manualDuty_ == 0) {
            return 0.0f;
        }
        return higherOf(manualDuty_, percentToDuty(settings_.minPercent));
    }

    float target = lastCurveDuty_;
    if (sensorOk_) {
        target = lastCurveDuty_ = curveDuty();
    } else if (sensorFault(now)) {
        target = percentToDuty(settings_.failsafePercent);
    }

    if (quietActive_) {
        target = lowerOf(target, percentToDuty(settings_.quietMaxPercent));
    }
    return target;
}

float FanController::kickDuty() const {
    if (!manual_ && quietActive_) {
        return percentToDuty(settings_.quietMaxPercent);
    }
    return FULL_DUTY;
}

uint8_t FanController::update(uint32_t now) {
    const float target = targetDuty(now);
    const float minDuty = percentToDuty(settings_.minPercent);
    const float elapsedSeconds = (now - lastUpdate_) / 1000.0f;
    lastUpdate_ = now;

    if (kicking_) {
        if (target == 0.0f) {
            kicking_ = false;
            return 0;
        }
        if (now - kickStart_ < KICK_MS) {
            return static_cast<uint8_t>(kickDuty());
        }
        kicking_ = false;
        current_ = lowerOf(target, minDuty);
        return static_cast<uint8_t>(current_ + 0.5f);
    }

    if (current_ == 0.0f) {
        if (target == 0.0f) {
            return 0;
        }
        kicking_ = true;
        kickStart_ = now;
        return static_cast<uint8_t>(kickDuty());
    }

    const float step = RAMP_DUTY_PER_SECOND * elapsedSeconds;
    if (target == 0.0f) {
        // Below minimum speed the fan may stall, so stop instead of creeping down to zero.
        current_ = current_ - step <= minDuty ? 0.0f : current_ - step;
    } else if (target > current_) {
        current_ = lowerOf(current_ + step, target);
    } else {
        current_ = higherOf(current_ - step, target);
    }
    return static_cast<uint8_t>(current_ + 0.5f);
}
