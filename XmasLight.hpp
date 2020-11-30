#pragma once
#include "MillisTime.hpp"
#include "AnyStream.hpp"
#include "DimmableLight.hpp"

class XmasLight : public DimmableLight {
private:
    static const byte STATES_COUNT = 4;
    static const byte X_LED_COUNT = 3;

    uint16_t sequence[STATES_COUNT][X_LED_COUNT] = {
        {  4095,     0,     0  },   // Red
        {  4095,  1100,     0  },   // Yellow
        {     0,  4095,     0  },   // Green
        {     0,     0,  4095  }    // Blue
    };

    bool enabled = false;
    byte state = 0;
    MillisTimer timer;

public:
    void XmasStart(uint16_t ms) {
        enabled = true;
        timer.Start(ms);
    }

    void XmasStop() {
        enabled = false;
        UpdateOutput();
    }

    void XmasSwitch() {
        if (enabled)
            XmasStop();
        else 
            XmasStart(1000);
    }

    void XmasHandle() {
        if (enabled && timer.HasExpired()) {
            for (int l = 0; l < X_LED_COUNT; l++)
                pwm.setPWM(ledPins[l], 0, sequence[state][l]);

            if (++state >= STATES_COUNT)
                state = 0;

            timer.Continue();
        }
    }
};
