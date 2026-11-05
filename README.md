# 🏠 HeatBodyVentilator

Der HeatBodyVentilator als Heizkörper-Lüfter Einheit mit vollständiger Home Assistant Integration, Temperaturüberwachung und intelligenter Lüftersteuerung.

[![License](https://img.shields.io/badge/License-Custom-red.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32-green.svg)](https://www.espressif.com/en/products/socs/esp32)
[![Framework](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)

## ✨ Features

### 🌐 Web Interface
- **Responsive Design**: Mobile-optimierte Benutzeroberfläche
- **Sicherer Login**: Token-basierte Authentifizierung
- **Echtzeit-Dashboard**: Live-Anzeige aller Sensordaten
- **Einstellungsseite**: Konfiguration von WiFi, MQTT und Geräteparametern
- **Deutschsprachig**: Vollständig lokalisierte Benutzeroberfläche

### 📡 Home Assistant Integration
- **MQTT Auto-Discovery**: Automatische Erkennung in Home Assistant
- **Sensoren**:
  - 🌡️ Temperatur (KMeter-ISO Thermoelement)
  - 💨 Lüfter-Geschwindigkeit in %
  - 📶 WiFi Signal-Stärke
  - ⏱️ Uptime
- **Steuerung**:
  - 💨 Lüfter: Geschwindigkeit, An/Aus und Preset „auto“
  - 🌡️ Start-/Maximaltemperatur und Mindestgeschwindigkeit
  - 🌙 Nachtmodus An/Aus und Nacht-Maximum
  - 💡 LED An/Aus
  - 🔄 Neustart-Button
- **Diagnose**: ⚠️ Sensorfehler (Problem-Sensor)
- **Status**: Online/Offline Überwachung (alle Entitäten werden bei Ausfall „nicht verfügbar“)

### 🌡️ Temperaturüberwachung
- **KMeter-ISO Sensor**: Professionelle K-Type Thermoelement-Unterstützung
- **Temperatureinheiten**: Celsius und Fahrenheit
- **Interne Temperatur**: Zusätzlicher interner Sensor
- **Fehlerüberwachung**: Sensor-Statusmeldungen

### 💨 Intelligente Lüftersteuerung
- **PWM-Steuerung**: Präzise Geschwindigkeitsregelung (0-100%) mit 25 kHz (unhörbar für 4-Pin-PC-Lüfter)
- **Auto-Modus**: Temperaturbasierte automatische Regelung
- **Manueller Modus**: Schieberegler im Dashboard oder über Home Assistant; der Modus bleibt nach einem Neustart erhalten
- **Temperatur-Mapping**: Konfigurierbare Start- und Maximaltemperatur (0-120 °C)
- **Sanfter Lauf**: 2 °C Hysterese, einstellbare Mindestgeschwindigkeit, 1 s Anlaufstoß und langsames Hoch-/Runterregeln
- **Nachtmodus**: Begrenzt die Automatik zu festen Zeiten (Zeit per NTP, Zeitzone Mitteleuropa)
- **Notbetrieb**: Fällt der Temperatursensor länger als 30 s aus, läuft der Lüfter mit einer einstellbaren Notgeschwindigkeit; der Sensor wird alle 30 s neu gesucht

### 🔐 Sicherheit
- **Passwortschutz**: Sicherer Web-Zugang
- **Session-Management**: Token-basierte Sitzungen
- **Passwort-Änderung**: Über Web-Interface
- **Factory Reset**: Zurücksetzen auf Werkseinstellungen

### 📶 WiFi Management
- **Station Mode**: Verbindung zu bestehendem WiFi
- **Access Point Mode**: Eigener Hotspot für Setup
- **Dual Mode**: Gleichzeitige AP und STA Funktion
- **WiFi Scanner**: Verfügbare Netzwerke anzeigen
- **Verbindungsstatus**: Live-Überwachung

## 🔧 Hardware-Anforderungen

### Minimal
- **ESP32**: M5Stack Atom oder kompatibles Board
- **Stromversorgung**: USB 5V

### Empfohlen
- **ESP32**: M5Stack Atom Lite
- **Temperatursensor**: M5Stack KMeter-ISO Unit (K-Type Thermoelement)
- **Lüfter**: 12V PWM-Lüfter (Pin 22)
- **LED**: FastLED-kompatible RGB LED (integriert bei M5Stack Atom Lite)

## 📦 Installation

### Voraussetzungen
- [VS Code](https://code.visualstudio.com/)
- [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)
- USB-Treiber für ESP32

### Schritt 1: Repository klonen
```bash
git clone https://github.com/smarthome-assistant/ESP32_Server_Project_Core.git
cd ESP32_Server_Project_Core
```

### Schritt 2: Projekt in VS Code öffnen
```bash
code .
```

### Schritt 3: Build und Upload
1. Öffnen Sie PlatformIO Home (Alien-Icon in der Sidebar)
2. Klicken Sie auf "Build" (✓)
3. Verbinden Sie Ihr ESP32 via USB
4. Klicken Sie auf "Upload" (→)

### Schritt 4: Erste Konfiguration
1. ESP32 startet einen Access Point: `SmartHome-Assistant-AP`
2. Verbinden Sie sich mit diesem WiFi (Passwort: `smarthome-assistant.info`)
3. Öffnen Sie im Browser: `http://192.168.4.1`
4. Login mit Standard-Passwort: `smarthome-assistant.info` (überschreibbar über `SECRET_WEB_PASSWORD` in `include/secrets.h`)
5. Konfigurieren Sie WiFi und MQTT

**Alternative: WiFi direkt in der Firmware hinterlegen**
1. Kopieren Sie `include/secrets.example.h` nach `include/secrets.h`
2. Tragen Sie SSID und Passwort ein (`secrets.h` wird von Git ignoriert)
3. Build und Upload wie in Schritt 3

Im Webinterface gespeicherte Zugangsdaten haben Vorrang vor `secrets.h`. Die Werte liegen unverschlüsselt in der Firmware, geben Sie die `.bin`-Datei daher nicht weiter.

## ⚙️ Konfiguration

### WiFi Einstellungen
- **SSID**: Ihr WiFi-Netzwerkname
- **Passwort**: Ihr WiFi-Passwort
- **Access Point**: Optional zusätzlicher AP-Modus

### MQTT Einstellungen
- **Server**: MQTT Broker IP/Hostname
- **Port**: Standard 1883
- **Username**: Optional
- **Password**: Optional
- **Topic**: Base-Topic für Nachrichten

### Lüftersteuerung
- **Start-Temperatur**: Temperatur ab der der Lüfter startet
- **Max-Temperatur**: Temperatur bei voller Geschwindigkeit
- **Mindestgeschwindigkeit**: Langsamste Stufe, bei der der Lüfter noch sicher dreht (Standard 10 %)
- **Notbetrieb**: Geschwindigkeit bei ausgefallenem Sensor (Standard 30 %)
- **Nachtmodus**: Beginn, Ende und Maximum (Standard 22:00–06:00, 30 %)

## 🏗️ Projektstruktur

```
ESP32_Server_Project_Core/
├── data/                      # Web-Dateien (HTML, CSS, Bilder)
│   ├── index.html            # Login-Seite
│   ├── login.html            # Login-Formular
│   ├── main.html             # Haupt-Dashboard
│   ├── setting.html          # Einstellungen
│   ├── style.css             # Einheitliche Styles
│   └── img/
│       └── logo.png          # Logo
├── src/                       # Quellcode
│   ├── main.cpp              # Hauptprogramm
│   ├── Config.*              # Konfigurationsverwaltung
│   ├── ServerManager.*       # Webserver-Management
│   ├── WiFiManager.*         # WiFi-Verbindungsverwaltung
│   ├── MQTTManager.*         # MQTT-Funktionalität
│   ├── LEDManager.*          # LED-Steuerung
│   ├── FanManager.*          # Lüfter: PWM, Modus, Nachtmodus, MQTT-Status
│   └── KMeterManager.*       # Temperatursensor-Management
├── lib/DeviceLogic/          # Hardwareunabhängige Logik (Lüfterkurve, Token, WiFi-Wiederverbindung)
├── test/                     # Unit-Tests dafür: pio test -e native
├── platformio.ini            # PlatformIO-Konfiguration
└── README.md                 # Diese Datei
```

## 📚 Libraries

- **FastLED** 3.9.20 - LED-Steuerung
- **ArduinoJson** 6.21.5 - JSON-Verarbeitung
- **PubSubClient** 2.8.0 - MQTT-Client
- **M5Unit-KMeterISO** 1.0.1 - Temperatursensor
- **Preferences** - ESP32 NVS Storage
- **WiFi** - ESP32 WiFi
- **WebServer** - ESP32 Webserver
- **Update** - OTA Updates

## 🐛 Troubleshooting

### ESP32 startet nicht
- Drücken Sie den Reset-Button
- Überprüfen Sie die Stromversorgung
- Versuchen Sie einen Factory Reset

### Keine WiFi-Verbindung
- Überprüfen Sie SSID und Passwort
- Stellen Sie sicher, dass 2.4GHz WiFi verwendet wird
- Aktivieren Sie den AP-Modus für Neukonfiguration

### MQTT verbindet nicht
- Überprüfen Sie Broker-IP und Port
- Testen Sie die Verbindung mit MQTT Explorer
- Überprüfen Sie Username/Password (falls verwendet)

### Sensor zeigt keine Daten
- Überprüfen Sie die I2C-Verbindung
- Stellen Sie sicher, dass KMeter-ISO korrekt angeschlossen ist
- Prüfen Sie die Sensor-Adresse (Standard: 0x66)

## 🚀 Erweiterte Funktionen

### OTA Updates
- Firmware-Updates über Web-Interface

### API Endpoints
Das Gerät bietet REST-APIs für externe Integration (alle benötigen den Login-Token als `?token=`):
- `GET /api/wifi-status` - WiFi-Status
- `GET /api/mqtt-status` - MQTT-Status
- `POST /api/pwm-control` - Lüftergeschwindigkeit setzen (`duty` 0-255, schaltet in den manuellen Modus)
- `POST /api/auto-pwm` - Automatik an/aus (`enabled` 1/0)
- `GET /api/pwm-status` - Lüfterstatus inkl. Modus, Sensorfehler und Nachtmodus
- `GET`/`POST /api/fan-settings` - Mindestgeschwindigkeit, Notbetrieb, Nachtmodus
- `GET /api/kmeter-status` - Temperaturdaten

### Tests
Die hardwareunabhängige Logik wird auf dem Rechner getestet:
```bash
pio test -e native
```

## 📄 Lizenz

### Eigener Code (Custom Source-Available License)

Der eigene Programmcode dieses Projekts (alle Dateien in `/src` und `/data`) unterliegt einer **Custom Source-Available License**:

- ✅ **Erlaubt**: Private und nicht-kommerzielle Nutzung, Bildungszwecke, Studium des Codes
- ❌ **Nicht erlaubt ohne Genehmigung**: Kommerzielle Nutzung, Verkauf, Veröffentlichung modifizierter Versionen
- 📄 **Details**: Siehe [LICENSE](LICENSE) Datei

**Für kommerzielle Nutzung** kontaktieren Sie bitte:
- GitHub: [@smarthome-assistant](https://github.com/smarthome-assistant)
- Email: markus.reza.tain@smarthome-assistant.info

### Verwendete Open-Source Bibliotheken

Dieses Projekt nutzt folgende **MIT-lizenzierte** Open-Source-Bibliotheken, die kommerzielle Nutzung erlauben:

| Bibliothek | Lizenz | Zweck |
|------------|--------|-------|
| [FastLED](https://github.com/FastLED/FastLED) | MIT | LED-Steuerung |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | MIT | JSON-Verarbeitung |
| [PubSubClient](https://github.com/knolleary/pubsubclient) | MIT | MQTT-Client |
| [M5Unit-KMeterISO](https://github.com/m5stack/M5Unit-KMeterISO) | MIT | Temperatursensor |

📋 **Vollständige Lizenztexte**: Siehe [THIRD_PARTY_LICENSES.md](THIRD_PARTY_LICENSES.md)

---

**Hinweis**: Die restriktive Lizenz gilt NUR für den eigenen Code. Die verwendeten Bibliotheken bleiben unter ihren permissiven MIT-Lizenzen verfügbar.

## �👤 Autor

**SmartHome-Assistant.info (Markus Reza Tain)**
- GitHub: [@smarthome-assistant](https://github.com/smarthome-assistant)
- Repository: [HeatBodyVentilator](https://github.com/smarthome-assistant/HeatBodyVentilator)

**Kontakt für Lizenzanfragen:**
- Kommerzielle Nutzung
- Veröffentlichung modifizierter Versionen
- Sondervereinbarungen

## 🙏 Danksagungen

- M5Stack für die hervorragende Hardware
- Home Assistant Community
- PlatformIO Team

## 📝 Changelog

### Version 0.0.3 (2026-11-05)
- 💨 Lüftersteuerung in Home Assistant (Lüfter-Entität, Zahlen für Temperaturen und Mindestgeschwindigkeit) und im Dashboard (Automatik-Schalter, Schieberegler)
- 🔇 PWM auf 25 kHz, Hysterese, Mindestgeschwindigkeit, Anlaufstoß und sanftes Regeln
- 🌙 Nachtmodus mit Geschwindigkeitsgrenze; Zeitzone jetzt mit Sommerzeit
- ⚠️ Notbetrieb und Sensorfehler-Meldung bei ausgefallenem Temperatursensor
- 📡 Verfügbarkeit für alle Home-Assistant-Entitäten
- 🔐 Behoben: Anmeldung mit leerem Token möglich; Session-Token jetzt 128 Bit; `/api/status` und `/api/settings` erfordern Login
- 🐛 Behoben: Neustart-Knöpfe, MQTT-Passwort wurde beim Speichern mit Sternchen überschrieben, Maximaltemperatur über 70 °C abgelehnt
- 📶 Behoben: WiFi-Wiederverbindung blockierte die Lüftersteuerung und schaltete den Notfall-AP ab

### Version 0.0.2 (2025-11-11)
- ⚡ Optimierte PWM-Frequenz für verbesserte Lüftersteuerung

### Version 0.0.1 (2025-10-21)
- ✨ Initiale Entwicklungs-Version
- 🌡️ KMeter-ISO Temperatursensor-Integration
- 💨 Intelligente Lüftersteuerung mit Auto-Modus
- 📡 Vollständige Home Assistant MQTT Integration
- 🌐 Responsive Web-Interface
- 🔐 Sicherheitsfunktionen implementiert

---

**Hinweis**: Dies ist ein Community-Projekt. Beiträge sind willkommen!
