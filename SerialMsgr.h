#pragma once
#include <Arduino.h>
#include "Timer.h"

#define PC_SERIAL 0
#define BT_SERIAL 1
#define WIFI_SERIAL 2


class SerialMsgr {
public:
    static String msgHistory;
    static HardwareSerial* serials[4];

    static void Initialize() {
        serials[PC_SERIAL] = &Serial;
        serials[BT_SERIAL] = &Serial3;
        serials[WIFI_SERIAL] = &Serial1;

        serials[PC_SERIAL]->begin(115200);
        serials[WIFI_SERIAL]->begin(115200);
        serials[BT_SERIAL]->begin(9600);
    }

    static String ReadMsg(int s) {
        String msg = "";

        if (s == WIFI_SERIAL) {
            msg = ReadWifiMsg();
        }
        else {
            while (serials[s]->available()) {
                char c = serials[s]->read();
                if (c == '\n' || c == '`')
                    break;
                msg += c;

                if (!serials[s]->available())
                    delay(5);
            }
        }

        msg.trim();

        if (msg.length() > 0) {
            msg.toLowerCase();
            msgHistory += "[" + String(s) + "] " + msg + "\n";
            while (msgHistory.length() > 200) {
                msgHistory.remove(0, msgHistory.indexOf('\n') + 1);
            }
        }

        return msg;
    }

    static String ReadWifiMsg() {
        String msg = "";
        MillisTimer timeoutTimer(5000L);
        
        while (!timeoutTimer.HasExpired()) {
            while (serials[WIFI_SERIAL]->available()) {
                char c = serials[WIFI_SERIAL]->read();
                if (c == '`') 
                    return msg;
                msg += c;
            }
        }

        return "";
    }

    static void SendMsg(int s, String msg) {
        if (s == WIFI_SERIAL) {
            SendWifiMsg(msg);
        }
        else {
            serials[s]->println(msg);
        }
    }

    static void SendWifiMsg(String msg) {
        serials[WIFI_SERIAL]->print(msg + "`");
    }
};


String SerialMsgr::msgHistory;
HardwareSerial* SerialMsgr::serials[4];