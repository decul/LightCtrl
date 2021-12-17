#pragma once
#include "ArduinoSafe.hpp"
#ifndef C_CPP 
    #include <Adafruit_PWMServoDriver.h>
#endif
    #include <Wire.h>

#define COLOR_COUNT 5
#define PWM_RANGE 4095
// #define LINEAR_COMPONENT 1694
// #define FOURTH_COMPONENT 7
#define LINEAR_COMPONENT 1070
#define SQUARE_COMPONENT 55


class Light {
protected:
    const byte ledPins[COLOR_COUNT] = { 8, 11, 12, 15, 7 };

    Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

    float lightColor[COLOR_COUNT] = { 0.0, 0.0, 0.0, 1.0, 0.0 };
    float brightness = 1.0f;
    bool powerOn = false;

    virtual void OnColorChange() { }
    
    float Limit(float value);

public:
    Light();
    
    virtual void UpdateOutput();

    void SetColor(byte index, float value);
    void SetColors(float* rgbwy, float brightness);
    void SetColors(String* rgbwy, String brightness = "");
    void SetColors(float r, float g, float b, float w, float y);

    String GetColors();

    String GetOutputs();
    uint16_t GetOutput(byte l);
    uint16_t SetOutput(byte index, uint16_t value);

    void Power(bool on);
    void Switch();
};