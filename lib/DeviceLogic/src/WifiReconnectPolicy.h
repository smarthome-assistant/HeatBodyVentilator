#pragma once
#include <stdint.h>

struct WifiActions {
    bool reconnect = false;
    bool startAp = false;
    bool stopAp = false;
};

// Decides what to do about a lost connection, one short step at a time, so the
// main loop (and with it the fan control) never has to wait for WiFi.
class WifiReconnectPolicy {
public:
    WifiActions update(uint32_t now, bool connected, bool apRunning, bool apSetting) {
        WifiActions actions;

        if (connected) {
            down_ = false;
            actions.stopAp = apRunning && !apSetting;
            return actions;
        }

        if (!down_) {
            down_ = true;
            downSince_ = now;
            lastAttempt_ = now;
            actions.reconnect = true;
        } else if (now - lastAttempt_ >= RETRY_MS) {
            lastAttempt_ = now;
            actions.reconnect = true;
        }

        actions.startAp = !apRunning && now - downSince_ >= EMERGENCY_AP_DELAY_MS;
        return actions;
    }

private:
    static const uint32_t RETRY_MS = 30000;
    static const uint32_t EMERGENCY_AP_DELAY_MS = 30000;

    bool down_ = false;
    uint32_t downSince_ = 0;
    uint32_t lastAttempt_ = 0;
};
