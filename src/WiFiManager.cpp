#include "WiFiManager.h"
#include "WifiReconnectPolicy.h"
#include <EEPROM.h>
#include <Preferences.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef SECRET_WIFI_SSID
#define SECRET_WIFI_SSID ""
#endif
#ifndef SECRET_WIFI_PASSWORD
#define SECRET_WIFI_PASSWORD ""
#endif

WiFiManager wifi;
static WifiReconnectPolicy reconnectPolicy;

void WiFiManager::beginAP() {
    Serial.println("DEBUG: Starting Access Point...");
    Serial.println("DEBUG: AP_ENABLED = " + String(Config::AP_ENABLED));
    
    WiFi.mode(WIFI_AP_STA);
    bool result = WiFi.softAP(Config::AP_SSID, Config::AP_PASS);
    
    if (result) {
        Serial.println("Access Point started successfully");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
        Serial.print("AP SSID: ");
        Serial.println(Config::AP_SSID);
        Serial.print("Number of connected stations: ");
        Serial.println(WiFi.softAPgetStationNum());
    } else {
        Serial.println("ERROR: Failed to start Access Point!");
    }
}

void WiFiManager::stopAP() {
    Serial.println("Stopping Access Point...");
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    Serial.println("Access Point stopped - WiFi mode set to STA only");
}

bool WiFiManager::beginSTA(const char* ssid, const char* pass) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, pass);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Could not connect to WiFi. Starting emergency Access Point...");
        WiFi.disconnect();
        WiFi.softAP(Config::AP_SSID, Config::AP_PASS);
        return false;
    }

    Serial.println("Connected to WiFi!");
    Serial.println("Station IP Address: " + WiFi.localIP().toString());
    return true;
}

bool WiFiManager::loadCredentials(String& ssid, String& pass) {
    ssid = "";
    pass = "";

    Preferences prefs;
    if (prefs.begin("wifi", true)) {
        ssid = prefs.getString("ssid", "");
        pass = prefs.getString("password", "");
        prefs.end();
    }

    if (ssid.length() == 0 || pass.length() == 0) {
        ssid = SECRET_WIFI_SSID;
        pass = SECRET_WIFI_PASSWORD;
    }

    return ssid.length() > 0 && pass.length() > 0;
}

void WiFiManager::beginAutoSTA() {
    String savedSsid, savedPassword;

    if (loadCredentials(savedSsid, savedPassword)) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(savedSsid.c_str(), savedPassword.c_str());
        
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(500);
            Serial.println("Connecting to saved WiFi...");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected to saved WiFi!");
            Serial.println("Station IP Address: " + WiFi.localIP().toString());
        } else {
            Serial.println("Could not connect to saved WiFi.");
            Serial.println("Starting emergency Access Point for initial setup/troubleshooting...");
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(Config::AP_SSID, Config::AP_PASS);
            Serial.println("Emergency AP started - connect to " + String(Config::AP_SSID) + " network");
            Serial.print("Emergency AP IP: ");
            Serial.println(WiFi.softAPIP());
        }
    } else {
        Serial.println("No saved WiFi credentials found. Starting in AP mode for initial setup.");
        beginAP();
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::checkWiFiConnection() {
    bool connected = WiFi.status() == WL_CONNECTED;
    bool apRunning = (WiFi.getMode() & WIFI_MODE_AP) != 0;
    WifiActions actions = reconnectPolicy.update(millis(), connected, apRunning, Config::AP_ENABLED);

    if (actions.stopAp) {
        Serial.println("WiFi is back. Stopping emergency Access Point...");
        stopAP();
    }

    if (actions.startAp) {
        Serial.println("WiFi still down. Starting emergency Access Point for user access...");
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(Config::AP_SSID, Config::AP_PASS);
        Serial.println("Emergency AP started - user can access via " + String(Config::AP_SSID) + " network");
        Serial.print("Emergency AP IP: ");
        Serial.println(WiFi.softAPIP());
    }

    if (actions.reconnect) {
        Serial.println("WiFi connection lost. Status: " + String(WiFi.status()));
        Serial.println("Attempting to reconnect...");

        String savedSsid, savedPassword;

        if (loadCredentials(savedSsid, savedPassword)) {
            // WiFi.begin() returns immediately; the result shows up in a later check.
            WiFi.disconnect();
            WiFi.begin(savedSsid.c_str(), savedPassword.c_str());
        }
    }

    if (connected) {
        static unsigned long lastIPOutput = 0;
        if (millis() - lastIPOutput > 60000) {
            Serial.println("WiFi connected. IP: " + WiFi.localIP().toString());
            lastIPOutput = millis();
        }
    }
}

