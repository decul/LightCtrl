#pragma once
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <MillisTime.h>
#include "MyWiFiClient.h"


#define requestTimeout 5000
#define connectionTimeout 20000

#define SSID "Karuzela"
#define PASS "Programisci15k"
#define PORT 80


class WiFiMsgr {
private:
    static bool connected;
    static MillisTimer connectionTimer;

    static WiFiServer server;

public:
    static void CheckConnection() {
        if (connected) {
            if (WiFi.status() != WL_CONNECTED) {
                connected = false;
                Connect();
            }
        }
        else {
            if (WiFi.status() == WL_CONNECTED) {
                connected = true;
            }
            else if (connectionTimer.HasExpired()) {
                Connect();
            }
        }
    }

    static void Connect() {
        if (String(PASS).length() > 0)
            WiFi.begin(SSID, PASS);
        else 
            WiFi.begin(SSID);
        connectionTimer.Start(connectionTimeout);
    }

    static void Disconnect() {
        if (WiFi.status() == WL_CONNECTED) {
            WiFi.disconnect();
        }
    }

    static void Initialize() {
        //Prevents reconnection issue (taking too long to connect)
        WiFi.mode(WIFI_OFF);        
        delay(1000);
        WiFi.mode(WIFI_STA);
        Connect();
        server.begin();
    }

    static MyWiFiClient Client() {
        return server.available();
    }

    static String ReadMsg(MyWiFiClient &client) {
        String request = "";
        MillisTimer timeoutTimer(requestTimeout);

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

    static void SendResponse(MyWiFiClient &client, String response) {
        client.println("HTTP/1.1 200 OK");
        client.println("Access-Control-Allow-Origin: *");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();
        client.println(response);
        client.println();
        client.stop();
    }


};

WiFiServer WiFiMsgr::server = WiFiServer(PORT);
bool WiFiMsgr::connected = false;
MillisTimer WiFiMsgr::connectionTimer = MillisTimer();

