#pragma once
#include <EEPROM.h>

#define defaultColorAddr      0 // - 19     20B
#define defaultDimEndAddr    20 // - 21      2B

class MyEEPROM {

public:
    // commit 22 bytes of ESP8266 flash (for "EEPROM" emulation)
    // this step actually loads the content (22 bytes) of flash into 
    // a 22-byte-array cache in RAM
    //
    // EEPROM.commit() writes the content of byte-array cache to
    // hardware flash.  flash write occurs if and only if one or more byte
    // in byte-array cache has been changed, but if so, ALL 22 bytes are 
    // written to flash
    static void Initialize() {
        EEPROM.begin(22);
    }

    // static float GetDefaultColor(byte led) {
    //     float value;
    //     EEPROM.get(defaultColorAddr + led * 4, value);
    //     return value;
    // }
    // static void SetDefaultColor(byte led, float value) {
    //     if (led < 5)
    //         EEPROM.put(defaultColorAddr + led * 4, value);
    //     EEPROM.commit();
    // }

    // static Time GetDefaultDimEndTime() {
    //     byte h = EEPROM.read(defaultDimEndAddr);
    //     byte m = EEPROM.read(defaultDimEndAddr + 1);
    //     return Time(h, m);
    // }
    // static void SetDefaultDimEndTime(Time time) {
    //     EEPROM.write(defaultDimEndAddr, time.Hour());
    //     EEPROM.write(defaultDimEndAddr + 1, time.Min());
    //     EEPROM.commit();
    // }
};
