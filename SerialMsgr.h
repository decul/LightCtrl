#pragma once
#include <Arduino.h>
#include "Timer.h"

#define PC_SERIAL 0
#define BT_SERIAL 1
#define WIFI_SERIAL 2


class SerialMsgr {
public:
    static HardwareSerial* serials[4];

    static void Initialize() {
        serials[PC_SERIAL] = &Serial;
        serials[BT_SERIAL] = &Serial3;
        serials[WIFI_SERIAL] = &Serial1;

        serials[PC_SERIAL]->begin(115200);
        //serials[WIFI_SERIAL]->begin(115200);
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
        msg.toLowerCase();

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

HardwareSerial* SerialMsgr::serials[4];




String handleCommand(String cmd);

void handleSerialEvent(byte s) {
    String command = SerialMsgr::ReadMsg(s);
    String response = handleCommand(command);
    if (response.length() > 0)
        SerialMsgr::SendMsg(s, response);
}

void serialEvent() { 
    handleSerialEvent(PC_SERIAL);
}

void serialEvent1 () { 
    handleSerialEvent(WIFI_SERIAL);
}

void serialEvent3 () { 
    handleSerialEvent(BT_SERIAL);
}
