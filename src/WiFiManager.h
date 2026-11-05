#pragma once
#include <WiFi.h>
#include "Config.h"

class WiFiManager {
public:
    void beginAP();
    void stopAP();
    bool beginSTA(const char* ssid, const char* pass);
    void beginAutoSTA();
    bool loadCredentials(String& ssid, String& pass);
    bool isConnected();
    void checkWiFiConnection();
};
extern WiFiManager wifi;
