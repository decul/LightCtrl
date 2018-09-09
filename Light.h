#pragma once

class Light {
private:
    const int ledPins[4] = { 5, 6, 9, 10 };
    //int ledValues[4] = { 5, 0, 0, 0 };
    int ledMin[4] = { 25, 23, 24, 25 };
    int ledMax[4] = { 187, 176, 198, 225 };
    int ledTreshold[4] = { 25, 23, 24, 25 };
    //int ledWeights[4] = { 255, 108, 56, 56 };
    double rgbwValues[4] = { 1.0, 0.15, 0.0, 1.0 };
    double hsvValues[3] = { 0.0, 1.0, 1.0 };
    
    bool normalized = false;
    bool powerOn = true;

public:
    Light();
    
    void UpdateOutput();
    void IncreaseRgb(int color);
    void DecreaseRgb(int color);
    void SwitchRgb(int color);
    void SetRgbw(double* rgbw);
    void ChangeHue(int sign);
    void ChangeValue(int sign);
    void Power(bool on);
};