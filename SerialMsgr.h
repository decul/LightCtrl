#pragma once
#include <Arduino.h>

#define MAX_CMD_ARGS_LEN 8


class SerialMsgr {
public:
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

    static byte SplitCommand(String input, String &cmd, String* args) {
        byte argsNo = 0;
        cmd = GetWord(input);

        while (input.length() > 0 && argsNo < MAX_CMD_ARGS_LEN) 
            args[argsNo++] = SerialMsgr::GetWord(input);

        return argsNo;
    }

};
