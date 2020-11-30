#pragma once
#include <MillisTime.hpp>
#include "StrobeLight.hpp"
#include "MyEEPROM.hpp"

class DimmableLight : public StrobeLight {
private:
    const static byte STATE_COUNT = 8;
    const static byte UPDATE_PERIOD = 1;

    const float daylightRed = 0.0f;
    const float daylightGreen = 0.84f;
    const float eveningGreen = 0.45f;
    const float dawnGreen = 0.4f;

    Time stateStartTimes[STATE_COUNT];
    DateTime stateEndTime;

    MillisTimer dimmerTimer = MillisTimer(UPDATE_PERIOD * 1000);
    bool dimmerDisabled = false;
    bool initialized = false;
    byte state = 0;
    
    DateTime now;
    float delta;


public:
    DimmableLight() : StrobeLight() {
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
        if (!dimmerTimer.HasExpired() || StrobeRunning() || !DateTime::IsSet()) 
            return;

        now = DateTime::Now();
    
        if (!initialized) {
            initialized = true;
            Time time = now.time();

            while (!time.IsBetween(stateStartTimes[state], StateEndTime(state)) && state < STATE_COUNT)
                state++;
            stateEndTime = now.ClosestDate(StateEndTime(state));

            // if (state < 4)
            //     SetColors({ 1.0f, dawnGreen, 0.0f, 0.0f, 0.0f });
            // else if (state < 5)
            //     SetColors({ daylightRed, daylightGreen, 1.0f, 1.0f, 1.0f });
            // else
            //     SetColors({ 1.0f, eveningGreen, 0.0f, 0.0f, 1.0f });
            // UpdateColors();
        }

        float secondsLeft = (stateEndTime - now).TotalSeconds();
        if (secondsLeft > 0.0)
            delta = UPDATE_PERIOD / (secondsLeft + UPDATE_PERIOD);
        else 
            delta = 1.0f;

        switch (state) {
            case 0:     // Dim to black
                for (int l = 0; l < 5; l++)
                    DimDown(l, 0.0f);
                UpdateOutput();
                break;

            case 1:     // Raise to blue
                DimUp(2, 1.0f);
                UpdateOutput();
                break;

            case 2:     // Raise to white-blue
                DimUp(2, 1.0f);
                DimUp(3, 1.0f);
                UpdateOutput();
                break;

            case 3:     // Raise to daylight
                DimUp(0, daylightRed);
                DimUp(1, daylightGreen);
                DimUp(2, 1.0f);
                DimUp(3, 1.0f);
                DimUp(4, 1.0f);
                UpdateOutput();
                break;

            case 5:     // Dim to evening light
                Dim(0, 1.0f);
                Dim(1, eveningGreen);
                DimDown(2, 0.0f);
                DimDown(3, 0.0f);
                UpdateOutput();
                break;

            case 7:     // Dim to dawn
                DimDown(1, dawnGreen);
                DimDown(2, 0.0f);
                DimDown(3, 0.0f);
                DimDown(4, 0.0f);
                UpdateOutput();
                break;
        }

        if (now > stateEndTime) {
            state = (state + 1) % STATE_COUNT;
            if (state == 1)
                brightness = 1.0f;\
            stateEndTime = now.ClosestDate(StateEndTime(state));
        }

        dimmerTimer.Continue();
    }

    void Dim(byte led, float value) {
        lightColor[led] = delta * value + (1.0f - delta) * lightColor[led];
    }

    void DimUp(byte led, float value) {
        if (value > lightColor[led])
            Dim(led, value);
    }

    void DimDown(byte led, float value) {
        if (value < lightColor[led])
            Dim(led, value);
    }



    void DimmerReset() {
        // brightness = 1.0f;
        // for (byte i = 0; i < COLOR_COUNT; i++) {
        //     lightColor[i] = MyEEPROM::GetDefaultColor(i);
        //     dimmerInitColor[i] = lightColor[i];
        // }
        // UpdateOutput();

        // dimmerDisabled = false;
        // dimmerFinished = false;

        // DateTime now = DateTime::Now();
        // dimmerResetTime = now.ClosestDate(Time(defDimmerResetHour));

        // Time dimEnd = MyEEPROM::GetDefaultDimEndTime();
        // dimmerEndTime = now.ClosestDate(dimEnd);
        // if (dimmerEndTime >= dimmerResetTime) 
        //     dimmerEndTime = now;

        // dimmerStartTime = dimmerEndTime - TimeSpan(0, defDimmerSpan, 0, 0);
    }

    void DimmerEnable() {
        dimmerDisabled = false;
    }

    void DimmerDisable() {
        dimmerDisabled = true;
    }



    Time& StateEndTime(byte stateIdx) {
        return stateStartTimes[(state + 1) % STATE_COUNT];
    }

    void SetColorAsDefault() {
        for (byte i = 0; i < COLOR_COUNT; i++) 
            MyEEPROM::SetDefaultColor(i, lightColor[i]);
    }
};