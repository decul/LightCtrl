#pragma once
#include <EEPROM.h>
#include "RTClib.h"

// 1 byte variables
#define onByDefaultAddr 0x100

// Up to 4 bytes
#define defaultDimEndAddr 0x400

// Longer variables
#define defaultColorAddr 0x800      //0x800 - 0x81f reserved

class MyEEPROM {

public:
    float GetDefaultColor(int led) {
        float value;
        EEPROM.get(defaultColorAddr + led * 4, value);
        return value;
    }
    void SetDefaultColor(int led, float value) {
        if (led < 8 && value != GetDefaultColor(led))
            EEPROM.put(defaultColorAddr + led * 4, value);
    }

    Time GetDefaultDimEndTime() {
        Time time;
        EEPROM.get(defaultDimEndAddr, time);
        return time;
    }
    void SetDefaultDimEndTime(Time time) {
        EEPROM.put(defaultDimEndAddr, time);
    }

    bool IsOnByDefault() {
        return EEPROM.read(onByDefaultAddr);
    }
    void SetOnByDefault(bool value) {
        EEPROM.write(onByDefaultAddr, value);
    }
};