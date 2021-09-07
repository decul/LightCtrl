#pragma once
#include <MillisTime.hpp>
#include <ESP8266HTTPClient.h>
#include "WiFiMsgr.hpp"


class Extras {
private:
    static byte dzbPingFailCount;
    static MillisTimer dzbPingTimer;

public:
    static bool PingDzb(bool retry = false) {
        dzbPingTimer.AddTime(30 * 60 * 1000);

        HTTPClient http;
        uint32_t start = millis();
        http.begin("http://dzb.oocs.pl/hello.html");
        int code = http.GET();
        if (code < 400) {
            uint32_t time = millis() - start;
            Logger::Debug("DzB Ping returned " + String(code) + " in " + String(time) + " ms");
            return true;
        }
        else {
            Logger::Info("DzB Ping Failed: " + String(code));
        }

        if (retry) {
            if (++dzbPingFailCount < 5) {
                dzbPingTimer.Start(5000);
            }
            else {
                Logger::Error("DzB Ping Failed");
                dzbPingFailCount = 0;
            }
        }

        return false;
    }

    static void CheckDzbPing() {
        if (dzbPingTimer.HasExpired() && WiFiMsgr::IsConnected()) {
            PingDzb(true);
        }
    }
};



byte Extras::dzbPingFailCount = 0;
MillisTimer Extras::dzbPingTimer(60000);