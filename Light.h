#pragma once
#include <Arduino.h>
#include "LedCharacteristics.h"
#include "Timer.h"
#include "RTClib.h"
#include "MyEEPROM.h"

#define COLOR_COUNT 5

class Light {
private:
    RTC_DS1307 rtc;
    MyEEPROM memory;

    LedCharacteristics characteristics;

    const byte binaryPin = 8;

    float lightColor[COLOR_COUNT];

    bool powerOn = true;

    bool strobeEnabled = false;
    MicrosTimer strobePeriodTimer;
    MicrosTimer strobeDurationTimer;

    DateTime dimmerResetTime;
    DateTime dimmerStartTime;
    DateTime dimmerEndTime;
    bool dimmerDisabled = false;
    bool dimmerFinished = false;
    float dimmerInitColor[COLOR_COUNT];

public:
    const byte ledPins[COLOR_COUNT] = { 2, 6, 10, 11, 9 };
    
    Light();
    
    void UpdateOutput();

    void Brighten();
    void Brighten(int index);
    void Darken();
    void Darken(int index);

    void Switch(int index);

    void SetColor(int index, float value);
    void SetColor(float* color, int count = COLOR_COUNT);

    float GetColor(int index);
    String GetColor();

    void SetOutput(int index, byte value);
    String GetOutput();
    byte GetOutput(int l);

    void Power(bool on);
    void SwitchPower();

    void StartStrobe(float width, float frequency);
    void HandleStrobe();
    void StopStrobe();

    void HandleAutoDimming();
    void ResetDimmer();
    void EnableDimmer();
    void DisableDimmer();
    void UpdateDimmer();

    void SetColorAsDefault();

};