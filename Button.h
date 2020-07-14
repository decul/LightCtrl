#pragma once
#include <MillisTime.h>

class Button {
    const byte buttonPin;
    const byte holdTime = 5;
    const byte chainTime = 150;
    const byte continuousTime = 10;

    MillisTimer refreshTimer = MillisTimer(0);

    byte state = 1;

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
    static const byte NO_ACTION = 0;
    static const byte CLICK = 1;
    static const byte HOLD = 2;
    static const byte CLICK_HOLD = 3;
    static const byte DOUBLE_CLICK = 4;
    static const byte DOWN = 11;
    static const byte UP = 12;

    Button(byte arduinoPin) 
        : buttonPin(arduinoPin) {
        pinMode(buttonPin, INPUT_PULLUP);
    }

    long prevClick = 0;

    byte GetAction() {
        if (refreshTimer.HasExpired()) {
            switch (state) {
                // Waiting for click
                case 1: 
                    if (pressed()) {
                        state = 2;
                        timer.Start(chainTime);
                        return DOWN;
                    }
                    break;

                // Waiting for release
                case 2:
                    if (released()) {
                        state = 4;
                        timer.Start(chainTime);
                        return UP;
                    }
                    else if (timer.HasExpired()) {
                        state = 3;
                        timer.AddTime(continuousTime);
                        return HOLD;
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
                        return HOLD;
                    }
                    break;

                // Waiting for 2nd click
                case 4:
                    if (pressed()) {
                        state = 5;
                        timer.Start(chainTime);
                        return DOWN;
                    }
                    else if (timer.HasExpired()) {
                        state = 1;
                        return CLICK;
                    }
                    break;

                // Waiting for 2nd release
                case 5:
                    if (released()) {
                        state = 7;
                        timer.Start(chainTime);
                        return UP;
                    }
                    else if (timer.HasExpired()) {
                        state = 6;
                        timer.AddTime(continuousTime);
                        return CLICK_HOLD;
                    }
                    break;

                // Holding after click
                case 6: 
                    if (released()) {
                        state = 1;
                        return UP;
                    }
                    else if (timer.HasExpired()) {
                        timer.Continue();
                        return CLICK_HOLD;
                    }
                    break;

                // Can be used for detecting tripple click. 
                // Now is used to delay DOUBLE_CLICK, because releasing in 5th state returns UP
                case 7:
                    state = 1;
                    return DOUBLE_CLICK;
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
