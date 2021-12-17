#pragma once
#include <MillisTime.hpp>
#include "SmoothLight.hpp"
#include "MyEEPROM.hpp"

class DimmableLight : public SmoothLight {

protected:
    const float daylightGreen = 0.84f;
    const float eveningGreen = 0.45f;
    const float duskGreen = 0.4f;


private:
    const static byte STATE_COUNT = 8;
    const static byte UPDATE_PERIOD = 1;

    Time stateStartTimes[STATE_COUNT];
    DateTime stateEndTime;

    MillisTimer dimmerTimer = MillisTimer(UPDATE_PERIOD * 1000);
    bool dimmerDisabled = false;
    bool initialized = false;
    byte state = 0;
    
    bool dimmerSkipped = false;
    DateTime dimmerSkipEndDate;

    DateTime now;
    float delta;

    byte zone;



    void Initialize() {
        Time time = now.time();

        while (!time.IsBetween(stateStartTimes[state], StateEndTime(state)) && state < STATE_COUNT)
            state++;
        stateEndTime = now.ClosestDate(StateEndTime(state));

        if (state < 4)
            SetDuskLight();
        else if (state < 5)
            SetDaylight();
        else
            SetEveningLight();
        UpdateOutput();
        
        initialized = true;
    }


    void DimLights() {
        float secondsLeft = (stateEndTime - now).TotalSeconds();
        if (secondsLeft > 0.0)
            delta = UPDATE_PERIOD / (secondsLeft + UPDATE_PERIOD);
        else 
            delta = 1.0f;

        switch (state) {
            case 0:     // Dim to black
                Dim(0.0f);
                break;

            case 1:     // Raise to blue
                Dim(1.0f);
                Dim(2, 1.0f);
                break;

            case 2:     // Raise to white-blue
                Dim(1.0f);
                Dim(2, 1.0f);
                Dim(3, 1.0f);
                break;

            case 3:     // Raise to daylight
                Dim(1.0f);
                Dim(0, 0.0f);
                Dim(1, daylightGreen);
                Dim(2, 1.0f);
                Dim(3, 1.0f);
                Dim(4, 1.0f);
                break;

            case 5:     // Dim to evening light
                Dim(0, 1.0f);
                Dim(1, eveningGreen);
                Dim(2, 0.0f);
                Dim(3, 0.0f);
                break;

            case 7:     // Dim to dusk
                Dim(0, 1.0f);
                Dim(1, duskGreen);
                Dim(2, 0.0f);
                Dim(3, 0.0f);
                Dim(4, 0.0f);
                break;
        }

        if (state != 4 && state != 6)
            UpdateOutput();
    }

    void Dim(byte led, float value) {
        lightColor[led] = delta * value + (1.0f - delta) * lightColor[led];
    }

    void Dim(float value) {
        brightness = delta * value + (1.0f - delta) * brightness;
    }

    void DimUp(byte led, float value) {
        if (value > lightColor[led])
            Dim(led, value);
    }

    void DimDown(byte led, float value) {
        if (value < lightColor[led])
            Dim(led, value);
    }


    Time& StateEndTime(byte stateIdx) {
        return stateStartTimes[(state + 1) % STATE_COUNT];
    }



public:
    DimmableLight() : SmoothLight() {
        stateStartTimes[0].Set(0);
        stateStartTimes[1].Set(6, 00);
        stateStartTimes[2].Set(6, 20);
        stateStartTimes[3].Set(6, 40);
        stateStartTimes[4].Set(7);
        stateStartTimes[5].Set(19);
        stateStartTimes[6].Set(20);
        stateStartTimes[7].Set(22);
    }


    void DimmerHandle() {
        if (!dimmerTimer.HasExpired() || !DateTime::IsSet()) 
            return;

        now = DateTime::Now();
    
        if (!initialized)
            Initialize();

        if (!dimmerSkipped && !dimmerDisabled && !StrobeRunning() && !transitionInProgress)
            DimLights();

        if (now > stateEndTime) {
            state = (state + 1) % STATE_COUNT;
            stateEndTime = stateEndTime.ClosestDate(StateEndTime(state));

            if (state == 1 && !dimmerSkipped && !dimmerDisabled) {
                SetColors(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
                brightness = 1.0f;
            }
            if (state == 3)
                DimmerEnable();
        }

        if (dimmerSkipped && now > dimmerSkipEndDate)
            dimmerSkipped = false;

        dimmerTimer.Continue();
    }


    void DimmerEnable() {
        dimmerDisabled = false;
    }

    void DimmerDisable() {
        dimmerDisabled = true;
    }

    void DimmerSkip(byte count) {
        dimmerSkipped = true;
        dimmerSkipEndDate = stateEndTime.ClosestDate(Time(7)) + TimeSpan(count - 1, 0, 0, 0);
    }


    void SetDaylight() {
        ColorTransition(0.0f, daylightGreen, 1.0f, 1.0f, 1.0f, 1.0f);
    }

    void SetEveningLight() {
        ColorTransition(1.0f, eveningGreen, 0.0f, 0.0f, 1.0f, 1.0f);
    }

    void SetDuskLight() {
        ColorTransition(1.0f, duskGreen, 0.0f, 0.0f, 0.0f, 1.0f);
    }

    void SwitchSmartColors() {
        if (lightColor[3] > 0.0) 
            SetEveningLight();
        else 
            SetDaylight();
    }


    void AdjustDimmer(float difference) {
        bool isRising = difference > 0.0f;
        bool limitReached = false;

        if (zone == 0) {
            if (isRising) {
                if (brightness < 0.99f)
                    zone = 1;
                else if (lightColor[4] < 0.99f)
                    zone = 2;
                else
                    zone = 3;
            }
            else {
                if (lightColor[3] > 0.0f)
                    zone = 3;
                else if (lightColor[4] > 0.0f)
                    zone = 2;
                else 
                    zone = 1;
            }
        }
        
        // Power(true);

        float value;
        switch (zone) {
            case 1:
                brightness = Limit(brightness + difference);
                break;

            case 2:
                value = Limit(lightColor[4] + difference);
                lightColor[1] = value * eveningGreen + (1.0f - value) * duskGreen;
                lightColor[4] = value;
                break;

            case 3:
                value = Limit(lightColor[3] + difference);
                lightColor[0] = 1.0 - value;
                lightColor[1] = value * daylightGreen + (1.0f - value) * eveningGreen;
                lightColor[2] = value;
                lightColor[3] = value;
                break;
        }


        if (zone == 1) {
            if (isRising) 
                limitReached = brightness == 1.0f;
        }
        else if (zone == 2) {
            if (isRising) 
                limitReached = lightColor[4] == 1.0f;
            else 
                limitReached = lightColor[4] == 0.0f;
        }
        else if (zone == 3) {
            if (isRising) 
                limitReached = lightColor[3] == 1.0f;
            else 
                limitReached = lightColor[3] == 0.0f;
        }

        if (limitReached) {
            pwm.setPWM(ledPins[4], 0, PWM_RANGE);
            delay(1);
            pwm.setPWM(ledPins[4], 0, 0);
            delay(1);
            zone = -1;
        }


        UpdateOutput();
        OnColorChange();
    }

    void ResetDimmerZone() {
        zone = 0;
    }
};