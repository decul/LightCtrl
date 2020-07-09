#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MillisTime.h>
#include "Logger.h"


#define REQUEST_TIMEOUT 5000
#define CONNECTION_TIMEOUT 25000
#define RECONNECT_DELAY 5000
//#define LED_BLINK_PERIOD 500

#define LAN_SSID "Karuzela"
#define LAN_PASS "Programisci15k"
#define PORT 80


class WiFiMsgr {
private:
    static wl_status_t cachedStatus;
    static MillisTimer connectionTimer;
    static MillisTimer statusTimer;
    static MillisTimer rssiTimer;

    static WiFiServer server;

public:
    static bool displayRssi;

    static void CheckConnection() {
        if (statusTimer.HasExpired()) {
            wl_status_t status = WiFi.status();
            if (cachedStatus != status) {
                Logger::Info("WiFi: " + Status(status));
                cachedStatus = status;
            }

            if (status == WL_IDLE_STATUS && connectionTimer.HasExpired()) {
                Connect();
            }
            else if (status != WL_CONNECTED && connectionTimer.HasExpired()) {
                WiFi.disconnect();
                connectionTimer.Start(RECONNECT_DELAY);
            }

            statusTimer.Continue();
        }

        if (displayRssi && rssiTimer.HasExpired()) {
            Serial.println(RSSI());
            rssiTimer.Restart();
        }
    }

    static void Connect() {
        if (String(LAN_PASS).length() > 0)
            WiFi.begin(LAN_SSID, LAN_PASS);
        else 
            WiFi.begin(LAN_SSID);
        connectionTimer.Start(CONNECTION_TIMEOUT);
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
                return request;
            }

            if (timeoutTimer.HasExpired()) 
                client.stop();
        }

        return "";
    }

    static void SendResponse(WiFiClient &client, String response) {
        client.println("HTTP/1.1 200 OK");
        client.println("Access-Control-Allow-Origin: *");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();
        client.println(response);
        client.println();
        client.stop();
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
            case WL_DISCONNECTED:       return "Disconnected";
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
bool WiFiMsgr::displayRssi = false;