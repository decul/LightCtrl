#pragma once
#include <Arduino.h>
#include <MillisTime.h>
#include "MyEEPROM.h"

#define COLOR_COUNT 5

class Light {
private:
    float lightColor[COLOR_COUNT];
    bool powerOn = false;

    byte strobeMode = 0;
    byte strobeColor = 0;
    MicrosTimer strobePeriodTimer;
    MicrosTimer strobeDurationTimer;
    MicrosTimer dimmerTimer = MicrosTimer(0);

    DateTime dimmerResetTime;
    DateTime dimmerStartTime;
    DateTime dimmerEndTime;
    bool dimmerDisabled = false;
    bool dimmerFinished = false;
    float dimmerInitColor[COLOR_COUNT];

public:
    const byte ledPins[COLOR_COUNT] = { D6, D7, D8, D2, D1 };
    
    Light();
    
    void UpdateOutput();

    void AdjustColor(byte index, float value);
    void SetColor(byte index, float value);
    void SetColors(float* color, byte count = COLOR_COUNT);
    void SetColors(String* rgbwy);

    float GetColor(byte index);
    String GetColors();

    String GetOutputs();
    uint16_t GetOutput(byte l);
    uint16_t SetOutput(byte index, uint16_t value);

    void Power(bool on);
    void Switch();

    void StartStrobe(float width, float frequency);
    void StartRGBStrobe(float frequency);
    void HandleStrobe();
    void StopStrobe();

    void HandleAutoDimming();
    void ResetDimmer();
    void EnableDimmer();
    void DisableDimmer();
    void UpdateDimmer();

    void SetColorAsDefault();

    void Flash(long us);

};