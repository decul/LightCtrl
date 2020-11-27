#pragma once
#include <Arduino.h>
#include <MillisTime.h>
#include "MyEEPROM.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define COLOR_COUNT 5

class Light {
private:
    Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

    float lightColor[COLOR_COUNT];
    float brightness = 1.0f;
    bool powerOn = false;

    byte strobeMode = 0;
    byte strobeColor = 0;
    MicrosTimer strobePeriodTimer;
    MicrosTimer strobeDurationTimer;

    MillisTimer dimmerTimer = MillisTimer(0);
    DateTime dimmerResetTime;
    DateTime dimmerStartTime;
    DateTime dimmerEndTime;
    bool dimmerDisabled = false;
    bool dimmerFinished = false;
    float dimmerInitColor[COLOR_COUNT];

public:
    const byte ledPins[COLOR_COUNT] = { 8, 11, 12, 15, 7 };
    
    Light();
    
    void UpdateOutput();

    void AdjustBrightness(float difference);
    void AdjustColor(byte index, float difference);
    void SetColor(byte index, float value);
    void SetColors(float* color, byte count = COLOR_COUNT);
    void SetColors(String* rgbwy, String brightness = "");

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