#pragma once
#include <stdint.h>

const float TEMP_MAPPING_LIMIT_C = 120.0f;

inline uint8_t percentToDuty(int percent) {
    if (percent <= 0) return 0;
    if (percent >= 100) return 255;
    return static_cast<uint8_t>((percent * 255 + 50) / 100);
}

inline uint8_t dutyToPercent(int duty) {
    if (duty <= 0) return 0;
    if (duty >= 255) return 100;
    return static_cast<uint8_t>((duty * 100 + 127) / 255);
}

inline bool isValidTempMapping(float startTemp, float maxTemp) {
    // Written so that NaN (which fails every comparison) is rejected.
    return startTemp >= 0.0f && maxTemp <= TEMP_MAPPING_LIMIT_C && startTemp < maxTemp;
}

// Start is inclusive, end exclusive. A window may cross midnight; equal times mean no window.
inline bool isInQuietHours(int minuteOfDay, int startMinute, int endMinute) {
    if (startMinute == endMinute) return false;
    if (startMinute < endMinute) {
        return minuteOfDay >= startMinute && minuteOfDay < endMinute;
    }
    return minuteOfDay >= startMinute || minuteOfDay < endMinute;
}

// The web form never receives the stored password, so a blank field means "keep it".
inline const char* resolveMqttPassword(const char* submitted, const char* user, const char* stored) {
    if (user[0] == '\0') return "";
    return submitted[0] != '\0' ? submitted : stored;
}
