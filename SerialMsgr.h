#pragma once
#include <Arduino.h>
#include "Timer.h"

#define TIMEOUT 5000L

#define PC_SERIAL 0
#define BT_SERIAL 1
#define WIFI_SERIAL 2

#define MSG_BEG '#'
#define MSG_END '$'

class SerialMsgr {
public:
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
                if (c == '\n')
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

    static String ReadWifiMsg(bool wait = false) {
        String msg = "";
        bool validStart = false;
        MillisTimer timeoutTimer(TIMEOUT);
        
        do {
            while (serials[WIFI_SERIAL]->available()) {
                char c = serials[WIFI_SERIAL]->read();
                if (c == MSG_BEG) {
                    msg = "";
                    validStart = true;
                }
                else if (c == MSG_END && validStart) {
                    return msg;
                }
                else {
                    msg += c;
                }
            }
        } while (!timeoutTimer.HasExpired() && (wait || validStart));

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
        serials[WIFI_SERIAL]->print(MSG_BEG + msg + MSG_END);
    }

    static void Debug(String msg) {
        SendWifiMsg("savedebug " + msg);
    }
};

HardwareSerial* SerialMsgr::serials[4];




String handleCommand(String cmd);

void handleSerialEvent(byte s) {
    SerialMsgr::Debug("Msg from " + String(s));
    String command = SerialMsgr::ReadMsg(s);
    SerialMsgr::Debug(command);
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
