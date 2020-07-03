#pragma once
#include <Arduino.h>

class SerialMsgr {
public:
    static void Initialize() {
        Serial.begin(115200);
    }

    static String ReadMsg() {
        String msg = "";

        while (Serial.available()) {
            char c = Serial.read();
            if (c == '\n')
                break;
            msg += c;

            if (!Serial.available())
                delay(5);
        }

        msg.trim();
        msg.toLowerCase();

        return msg;
    }
    
    static String GetWord(String &input) {
        String result;
        int spaceIndex = input.indexOf(' ');

        if (spaceIndex > -1) {
            result = input.substring(0, spaceIndex);
            input.remove(0, spaceIndex + 1);
        }
        else {
            result = input.substring(0, input.length());
            input.remove(0, input.length());
        }

        input.trim();
        result.trim();

        return result;
    }

};




String handleCommand(String cmd);

void serialEvent() { 
    String command = SerialMsgr::ReadMsg();
    String response = handleCommand(command);
    if (response.length() > 0)
        Serial.println(response);
}