#pragma once
#include <EEPROM.h>
#include "RTClib.h"

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

    float SetDefaultDimEndTime(Time time) {
        EEPROM.put(defaultDimEndAddr, time);
    }
};