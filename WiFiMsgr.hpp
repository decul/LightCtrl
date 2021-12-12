#pragma once
#include "ArduinoSafe.hpp"
#ifndef C_CPP 
    #include <ESP8266WiFi.h>
#endif
#include <MillisTime.hpp>
#include "Logger.hpp"


#define REQUEST_TIMEOUT 1000
#define CONNECTION_TIMEOUT 25000
#define RECONNECT_DELAY 2000


#define LAN_SSID "Karuzela"
#define LAN_PASS "Programisci15k"
#define HOTSPOT_SSID "Byzio-phone"
#define HOTSPOT_PASS "byziawifi"
#define PORT 80


class WiFiMsgr {
private:
    static wl_status_t cachedStatus;
    static MillisTimer connectionTimer;
    static MillisTimer statusTimer;
    static MillisTimer rssiTimer;
    static MillisTimer scanTimer;

    static WiFiServer server;

    static bool hotspotActive;

public:
    static bool displayRssi;

    static void CheckConnection() {
        if (statusTimer.HasExpired()) {
            wl_status_t status = WiFi.status();
            if (cachedStatus != status) {
                if (cachedStatus == WL_CONNECTED) {
                    Logger::Info("WiFi Disconnected");
                }
                else if (status == WL_CONNECTED) {
                    Logger::Info("WiFi Connected to " + WiFi.SSID());
                }
                else {
                    Serial.println("WiFi: " + Status(status));
                }
                cachedStatus = status;
            }
            
            if (status == WL_IDLE_STATUS && connectionTimer.HasExpired()) {
                Connect();
            }
            else if (status != WL_CONNECTED && connectionTimer.HasExpired()) {
                Reconnect();
            }

            statusTimer.Continue();
        }

        int8_t networkCount = WiFi.scanComplete();
        bool hotspotConnected = WiFi.SSID() == HOTSPOT_SSID;		// Do not execute it over and over again!

        bool shouldScan =  scanTimer.HasExpired()
                        && !hotspotConnected                // Not already connected to hotspot
                        && networkCount == -2               // No scan started yet
                        && cachedStatus != WL_DISCONNECTED; // WiFi not trying to connect

        if (shouldScan) {
            WiFi.scanNetworks(true);
            scanTimer.Continue();
        }
        if (networkCount >= 0) {
            hotspotActive = false;
            for (int i = 0; i < networkCount; i++) {
                if (WiFi.SSID(i) == HOTSPOT_SSID && !hotspotConnected) {
                    hotspotActive = true;
                    Reconnect();
                }
            }
            WiFi.scanDelete();
        }

        if (displayRssi && rssiTimer.HasExpired()) {
            Serial.println(RSSI());
            rssiTimer.Restart();
        }
    }

    static void Connect() {
        if (hotspotActive) {
            WiFi.begin(HOTSPOT_SSID, HOTSPOT_PASS);
        }
        else if (String(LAN_PASS).length() > 0) {
            WiFi.begin(LAN_SSID, LAN_PASS);
        }
        else  {
            WiFi.begin(LAN_SSID);
        }
        connectionTimer.Start(CONNECTION_TIMEOUT);
    }

    static void Reconnect() {
        WiFi.disconnect();
        connectionTimer.Start(RECONNECT_DELAY);
    }

    static void Initialize() {
        //Prevents reconnection issue (taking too long to connect)
        WiFi.mode(WIFI_OFF);        
        delay(1000);
        WiFi.mode(WIFI_STA);
        Connect();
        server.begin();
    }

    static WiFiClient Client() {
        if (IsConnected())
            return server.available();
        else 
            return WiFiClient();
    }

    static String ReadMsg(WiFiClient &client) {
        String request = "";
        MillisTimer timeoutTimer(REQUEST_TIMEOUT);

        while (client.connected()) {
            while (client.available()) {
                char c = client.read();
                if (c != '\r') 
                    request += c;
            }

            if (request.indexOf("\n\n") > 0) {
                request.remove(0, request.indexOf("GET /") + 5);
                request.remove(request.indexOf(" "));
                request.replace("/", " "); 
                request.trim();
                request.toLowerCase();
                return request;
            }

            if (timeoutTimer.HasExpired()) {
                client.println("HTTP/1.1 408");
                client.stop();
            }
        }

        return "";
    }

    static bool IsConnected() {
        return cachedStatus == WL_CONNECTED;
    }

    static String Status() {
        return Status(cachedStatus);
    }

    static String Status(wl_status_t status) {
        switch (status) {
            case WL_IDLE_STATUS:        return "Idle";
            case WL_NO_SSID_AVAIL:      return "No SSID Available";
            case WL_SCAN_COMPLETED:     return "Scan Completed";
            case WL_CONNECTED:          return "Connected";
            case WL_CONNECT_FAILED:     return "Connection Failed";
            case WL_CONNECTION_LOST:    return "Connection Lost";
            case WL_DISCONNECTED:       return "Connecting";
        }
    }

    static String RSSI() {
        int32_t signal = WiFi.RSSI();
        if (signal > 0)
            return "---";
        else 
            return String((WiFi.RSSI() + 100) * 2) + "%";
    }

};

WiFiServer WiFiMsgr::server = WiFiServer(PORT);
wl_status_t WiFiMsgr::cachedStatus = WL_DISCONNECTED;
MillisTimer WiFiMsgr::connectionTimer(CONNECTION_TIMEOUT);
MillisTimer WiFiMsgr::statusTimer(100);
MillisTimer WiFiMsgr::rssiTimer(500);
MillisTimer WiFiMsgr::scanTimer(5000);
bool WiFiMsgr::displayRssi = false;
bool WiFiMsgr::hotspotActive = false;