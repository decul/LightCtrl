#pragma once
#include <MillisTime.h>
#include "StrobeLight.h"
#include "MyEEPROM.h"

#define defDimmerSpan 2
#define defDimmerResetHour 10
#define dimmerMinPeriod 30


class DimmableLight : public StrobeLight {
private:
    MillisTimer dimmerTimer = MillisTimer(0);
    DateTime dimmerResetTime;
    DateTime dimmerStartTime;
    DateTime dimmerEndTime;
    bool dimmerDisabled = false;
    bool dimmerFinished = false;
    float dimmerInitColor[COLOR_COUNT];
    bool firstReset = true;


public:
    void OnColorChange() {
        StrobeLight::OnColorChange();
        DimmerUpdate();
    }

    void DimmerHandle() {
        if (DateTime::IsSet() && dimmerTimer.HasExpired() && !StrobeRunning()) {
            DateTime now = DateTime::Now();
        
            // Reset dimmer once a day
            if (now > dimmerResetTime && !powerOn || firstReset) {
                DimmerReset();
                firstReset = false;
            }

            // Dim light if that's right time
            if (!dimmerDisabled && !dimmerFinished && now > dimmerStartTime) {
                TimeSpan dimmerSpan = dimmerEndTime - dimmerStartTime;
                TimeSpan timeRemaining = dimmerEndTime - now;
                float percent = max(0.0f, (float)timeRemaining.TotalSeconds() / dimmerSpan.TotalSeconds());

                for (byte i = 2; i < COLOR_COUNT; i++) 
                    lightColor[i] = percent * dimmerInitColor[i];
                UpdateOutput();

                if (now > dimmerEndTime) 
                    dimmerFinished = true;
            }

            dimmerTimer.Start(1000);
        }
    }

    void DimmerReset() {
        brightness = 1.0f;
        for (byte i = 0; i < COLOR_COUNT; i++) {
            lightColor[i] = MyEEPROM::GetDefaultColor(i);
            dimmerInitColor[i] = lightColor[i];
        }
        UpdateOutput();

        dimmerDisabled = false;
        dimmerFinished = false;

        DateTime now = DateTime::Now();
        dimmerResetTime = now.ClosestDate(Time(defDimmerResetHour));

        Time dimEnd = MyEEPROM::GetDefaultDimEndTime();
        dimmerEndTime = now.ClosestDate(dimEnd);
        if (dimmerEndTime >= dimmerResetTime) 
            dimmerEndTime = now;

        dimmerStartTime = dimmerEndTime - TimeSpan(0, defDimmerSpan, 0, 0);
    }

    void DimmerEnable() {
        dimmerDisabled = false;
        DimmerUpdate();
    }

    void DimmerDisable() {
        dimmerDisabled = true;
    }

    void DimmerUpdate() {
        if (lightColor[2] == 0.0 && lightColor[3] == 0.0 && lightColor[4] == 0.0) {
            dimmerFinished = true;
            return;
        }

        for (byte i = 0; i < COLOR_COUNT; i++) 
            dimmerInitColor[i] = lightColor[i];
        dimmerFinished = false;

        DateTime now = DateTime::Now();
        if (now > dimmerStartTime) {
            dimmerStartTime = now;

            TimeSpan minPeriod = TimeSpan(0, 0, dimmerMinPeriod, 0);

            if (dimmerEndTime - now < minPeriod) {
                dimmerEndTime = now + minPeriod;
            }
        }
    }

    void SetColorAsDefault() {
        for (byte i = 0; i < COLOR_COUNT; i++) 
            MyEEPROM::SetDefaultColor(i, lightColor[i]);
    }
};