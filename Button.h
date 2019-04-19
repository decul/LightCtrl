#pragma once
#include "Timer.h"

class Button {
    const byte buttonPin;
    const byte holdTime = 5;
    const byte chainTime = 150;
    const byte continuousTime = 100;

    byte state = 1;

    MillisTimer timer;

    bool pressed() {
        if (digitalRead(buttonPin) == HIGH) {
            delay(holdTime);
            bool btn = digitalRead(buttonPin) == HIGH;
            if (!btn)
                Serial.println("Button press discarded");
            return btn;
        }
        return false;
    }

    bool released() {
        if (digitalRead(buttonPin) == LOW) {
            delay(holdTime);
            bool btn = digitalRead(buttonPin) == HIGH;
            if (btn)
                Serial.println("Button release discarded");
            return !btn;
        }
        return false;
    }

public:
    static const byte NO_ACTION = 0;
    static const byte SWITCH_POWER = 1;
    static const byte BRIGHTEN = 2;
    static const byte DARKEN = 3;

    Button(byte arduinoPin) 
        : buttonPin(arduinoPin) {
        pinMode(buttonPin, INPUT);
    }

    long prevClick = 0;

    byte GetAction() {
        switch (state) {
            // Waiting for click
            case 1: 
                if (pressed()) {
                    state = 2;
                    timer.Start(chainTime);
                }
                break;

            // Waiting for release
            case 2:
                if (timer.HasExpired()) {
                    state = 3;
                    timer.AddTime(continuousTime);
                    return BRIGHTEN;
                }
                else if (released()) {
                    state = 4;
                    timer.Start(chainTime);
                }
                break;

            // Increasing brightness
            case 3:
                if (timer.HasExpired()) {
                    timer.Continue();
                    return BRIGHTEN;
                }
                else if (released()) {
                    state = 1;
                }
                break;

            // Waiting for 2nd click
            case 4:
                if (timer.HasExpired()) {
                    state = 1;
                    return SWITCH_POWER;
                }
                else if (pressed()) {
                    state = 5;
                    timer.Start(continuousTime);
                    return DARKEN;
                }
                break;

            // Decreasing brightness
            case 5:
                if (timer.HasExpired()) {
                    timer.Continue();
                    return DARKEN;
                }
                else if (released()) {
                    state = 1;
                }
                break;

            default:
                state = 1;
                break;
        }

        return NO_ACTION;
    }
};
