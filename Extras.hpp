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
        HTTPClient http;
        uint32_t start = millis();
        http.begin("http://dzb-test.herokuapp.com/");
        int code = http.GET();
        if (code < 400 && code > 0) {
            uint32_t time = millis() - start;
            Logger::Debug("DzB Ping returned " + String(code) + " in " + String(time) + " ms");
            dzbPingFailCount = 0;
            return true;
        }
        else {
            Logger::Info("DzB Ping Failed: " + String(code));
        }

        if (retry) {
            dzbPingTimer.Start(60000);
            if (++dzbPingFailCount < 5) {
                Logger::Error("DzB Ping Failed Fivefold");
            }
        }

        return false;
    }

    static void CheckDzbPing() {
        if (dzbPingTimer.HasExpired() && WiFiMsgr::IsConnected()) {
            dzbPingTimer.AddMinutes(15);
            PingDzb(true);
        }
    }
};



byte Extras::dzbPingFailCount = 0;
MillisTimer Extras::dzbPingTimer(60000);