#pragma once
#include <Arduino.h>
#include "LedCharacteristics.h"

#define COLOR_COUNT 5

class Light {
private:
    LedCharacteristics characteristics;

    const byte ledPins[COLOR_COUNT] = { 2, 6, 10, 11, 9 };
    const byte binaryPin = 8;

    float lightColor[COLOR_COUNT] = { 0.0, 0.0, 0.0, 0.0, 0.5 };

    bool powerOn = true;

    bool strobeEnabled = false;
    long nextStrobeTime = 0;
    long prevStrobeTime = 0;
    int strobeDuration = 1000;
    int strobePeriod = 1000000;

public:
    Light();
    
    void UpdateOutput();

    void Brighten();
    void Brighten(int index);
    void Darken();
    void Darken(int index);

    void Switch(int index);

    void SetColor(int index, float value);
    void SetColor(float* color);

    float GetColor(int index);
    String GetColor();

    void SetOutput(int index, byte value);
    String GetOutput();
    byte GetOutput(int l);

    void Power(bool on);

    void StartStrobe(float duration, float period);
    void HandleStrobe();
    void StopStrobe();
};