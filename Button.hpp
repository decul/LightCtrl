#pragma once
#include <MillisTime.hpp>

class Button {
    const byte buttonPin;
    const byte holdTime = 5;
    const byte chainTime = 150;
    const byte continuousTime = 10;

    MillisTimer refreshTimer = MillisTimer(0);

    byte state = 1;
    byte clickCount = 0;

    MillisTimer timer;

    bool pressed() {
        if (digitalRead(buttonPin) == LOW) {
            delay(holdTime);
            return digitalRead(buttonPin) == LOW;
        }
        return false;
    }

    bool released() {
        if (digitalRead(buttonPin) == HIGH) {
            delay(holdTime);
            return digitalRead(buttonPin) == HIGH;
        }
        return false;
    }

public:
    // Least significant bit is set if button was held after last click
    // Other bits are the number of clicks
    static const byte NO_ACTION = 0;
    static const byte HOLD = 1;
    static const byte CLICK = 2;
    static const byte CLICK_HOLD = 3;
    static const byte DOUBLE_CLICK = 4;
    static const byte DOUBLE_CLICK_HOLD = 5;
    static const byte TRIPLE_CLICK = 6;
    static const byte TRIPLE_CLICK_HOLD = 7;
    static const byte QUADRUPLE_CLICK = 8;
    
    static const byte DOWN = 255;
    static const byte UP = 254;

    Button(byte arduinoPin) 
        : buttonPin(arduinoPin) {
        pinMode(buttonPin, INPUT_PULLUP);
    }

    byte GetAction() {
        if (refreshTimer.HasExpired()) {
            switch (state) {
                // Waiting for 1st click
                case 1: 
                    if (pressed()) {
                        clickCount = 0;
                        state = 2;
                        timer.Start(chainTime);
                        return DOWN;
                    }
                    break;

                // Waiting for release
                case 2:
                    if (released()) {
                        clickCount++;
                        state = 4;
                        timer.Start(chainTime);
                        return UP;
                    }
                    else if (timer.HasExpired()) {
                        state = 3;
                        timer.AddTime(continuousTime);
                        return (clickCount << 1) | 1;
                    }
                    break;

                // Holding
                case 3:
                    if (released()) {
                        state = 1;
                        return UP;
                    }
                    else if (timer.HasExpired()) {
                        timer.Continue();
                        return (clickCount << 1) | 1;
                    }
                    break;

                // Waiting for next click
                case 4:
                    if (pressed()) {
                        state = 2;
                        timer.Start(chainTime);
                        return DOWN;
                    }
                    else if (timer.HasExpired()) {
                        state = 1;
                        return (clickCount << 1) & 0xFE;
                    }
                    break;

                default:
                    state = 1;
                    break;
            }

            refreshTimer.Start(1);
        }

        return NO_ACTION;
    }
};
