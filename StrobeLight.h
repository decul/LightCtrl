#pragma once
#include "AnyStream.h"
#include "Light.h"

class StrobeLight : public Light {
private:
    byte strobeMode = 0;
    byte strobeColor = 0;
    MicrosTimer strobePeriodTimer;
    MicrosTimer strobeDurationTimer;


public:
    void UpdateOutput() {
        if (StrobeRunning())
            StrobeStop();
        Light::UpdateOutput();
    }

    void StrobeHardware(float width, uint16_t frequency) {
        Power(false);
        strobeMode = 1;
        pwm.setPWMFreq(frequency);
        for (int i = 0; i < COLOR_COUNT; i++)
            pwm.setPWM(ledPins[i], 0, PWM_RANGE * width);
    }

    void StrobeHardwareRGB(float width, uint16_t frequency) {
        Power(false);
        strobeMode = 1;
        pwm.setPWMFreq(frequency);
        for (int i = 0; i < 3; i++) {
            uint16_t start = PWM_RANGE * i / 3;
            uint16_t stop = (start + uint16_t(PWM_RANGE * width)) % (PWM_RANGE + 1);
            pwm.setPWM(ledPins[i], start, stop);
        }
        pwm.setPWM(ledPins[3], 0, 0);
        pwm.setPWM(ledPins[4], 0, 0);
    }



    // void StrobeSoftware(float width, float frequency) {
    //     Power(false);
    //     strobeMode = 1;
    //     float strobePeriod = 1000000.0 / frequency;
    //     strobePeriodTimer.Start(strobePeriod);
    //     strobeDurationTimer.SetPeriod(width * strobePeriod);
    // }

    // void StrobeSoftwareRGB(float frequency) {
    //     Power(false);
    //     strobeMode = 2;
    //     float strobePeriod = 1000000.0 / frequency;
    //     strobePeriodTimer.Start(strobePeriod);
    // }

    // void StrobeHandle() {
    //     if (strobeMode == 2) {
    //         Power(false);
    //         strobePeriodTimer.WaitForExpiration();
    //         strobeDurationTimer.Restart();
    //         //digitalWrite(ledPins[4], HIGH);
    //         strobeDurationTimer.WaitForExpiration();
    //         //digitalWrite(ledPins[4], LOW);
    //         strobePeriodTimer.Continue();
    //     }
    //     else if (strobeMode == 3) {
    //         Power(false);
    //         strobePeriodTimer.WaitForExpiration();
    //         //digitalWrite(ledPins[strobeColor], LOW);
    //         if (++strobeColor > 2)
    //             strobeColor = 0;
    //         //digitalWrite(ledPins[strobeColor], HIGH);
    //         strobePeriodTimer.Continue();
    //     }
    // }



    void StrobeStop() {
        strobeMode = 0;
        pwm.setPWMFreq(1600);
        Power(true);
    }

    bool StrobeRunning() {
        return strobeMode != 0;
    }



    void Flash(long us) {
        pwm.setPWM(ledPins[4], 0, PWM_RANGE);
        MicrosTimer::Sleep(us);
        pwm.setPWM(ledPins[4], 0, GetOutput(4));
    }



    static void StrobePrintHelpTo(AnyStream &stream) {
        stream.Println("width: 0.0 - 1.0");
        stream.Println("freq: 25 - 1600\n");
        stream.Println("Frequency precision:");
        stream.Println(" 25 -  79:   +-1");
        stream.Println(" 81 - 110:   +-2");
        stream.Println("111 - 180:   +-5");
        stream.Println("181 - 360:  +-20");
        stream.Println("361 - 800: +-100");
        stream.Println("    > 800: +-300");
    }
};