#pragma once
#include <Arduino.h>

#define COLOR_COUNT 5

class Light {
private:

    byte ledMin[COLOR_COUNT] = { 25, 23, 24, 25, 50 };
    byte ledMax[COLOR_COUNT] = { 187, 176, 198, 225, 255 };

    //const int ledWeights[COLOR_COUNT] = { 255, 108, 56, 56 };

    float lightColor[COLOR_COUNT] = { 0.0, 0.0, 0.0, 0.0, 0.5 };

    bool powerOn = true;

public:
    const byte ledPins[COLOR_COUNT] = { 2, 6, 10, 11, 9 };
    const byte binaryPin = 8;

    float alpha = 1.0;
    
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

    void Power(bool on);

    void SetAlpha(float alpha);
};