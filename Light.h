#pragma once
#include <Arduino.h>

class Light {
private:
    const byte ledPins[4] = { 5, 6, 9, 10 };

    byte ledMin[4] = { 25, 23, 24, 25 };
    byte ledMax[4] = { 187, 176, 198, 225 };

    //const int ledWeights[4] = { 255, 108, 56, 56 };

    float lightColor[4] = { 1.0, 0.40, 0.0, 1.0 };

    bool powerOn = true;

public:
    float alpha = 0.6;
    
    Light();
    
    void UpdateOutput();

    void Brighten();
    void Brighten(int index);
    void Darken();
    void Darken(int index);

    void Switch(int index);

    void SetColor(int index, float value);
    void SetColor(float* color);

    byte GetColor(int index);
    String GetColor();

    void Power(bool on);

    void SetAlpha(float alpha);
};