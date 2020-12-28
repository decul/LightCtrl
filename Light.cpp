#include "Light.hpp"
#include <math.h>


Light::Light() {
    Wire.pins(D6, D5);
    Wire.setClock(400000);
    pwm.begin();
    pwm.setPWMFreq(1600);
}


void Light::UpdateOutput() {
    for (byte l = 0; l < COLOR_COUNT; l++) 
        pwm.setPWM(ledPins[l], 0, GetOutput(l));
}


float Limit(float value) {
    return max(0.0f, min(1.0f, value));
}

void Light::AdjustBrightness(float difference) {
    Power(true);
    brightness = Limit(brightness + difference);
    UpdateOutput();
}

void Light::AdjustColor(byte index, float difference) {
    Power(true);
    lightColor[index] = Limit(lightColor[index] + difference);
    UpdateOutput();
    OnColorChange();
}


void Light::SetColor(byte index, float value) {
    lightColor[index] = Limit(value);
    UpdateOutput();
    OnColorChange();
}

void Light::SetColors(float* rgbwy, byte count) {
    for (byte l = 0; l < count; l++)
        lightColor[l] = Limit(rgbwy[l]);
    UpdateOutput();
    OnColorChange();
}

void Light::SetColors(String* rgbwy, String brightness) {
    if (brightness != "")
        this->brightness = brightness.toFloat();
    for (byte l = 0; l < COLOR_COUNT; l++)
        lightColor[l] = Limit(rgbwy[l].toFloat());
    UpdateOutput();
    OnColorChange();
}

void Light::SetColors(float r, float g, float b, float w, float y) {
    lightColor[0] = r;
    lightColor[1] = g;
    lightColor[2] = b;
    lightColor[3] = w;
    lightColor[4] = y;
    UpdateOutput();
    OnColorChange();
}

String Light::GetColors() {
    String str = "";
    for (byte l = 0; l < COLOR_COUNT; l++) 
        str += String(lightColor[l], 3) + " ";
    return str + String(brightness, 3);
}


void Light::Power(bool on) {
    if (powerOn != on) {
        powerOn = on;
        UpdateOutput();
    }
}
    
void Light::Switch() {
    Power(!powerOn);
}

float LuminosityFunc(float input) {
    float quartic = pow(FOURTH_COMPONENT * input, 4);
    float linear = LINEAR_COMPONENT * input;
    return quartic + linear;
}

uint16_t Light::GetOutput(byte l) {
    if (!powerOn || lightColor[l] == 0.0) {
        return 0;
    }
    else {
        float color = LuminosityFunc(lightColor[l]);
        float bright = LuminosityFunc(brightness) / PWM_RANGE;
        return color * bright;
    }
}

String Light::GetOutputs() {
    String str = "";
    for (byte l = 0; l < COLOR_COUNT; l++) 
        str += String(GetOutput(l)) + " ";
    return str;
}

uint16_t Light::SetOutput(byte index, uint16_t value) {
    float lower = 0.0f;
    float upper = 1.0f;
    float middle;

    for (int i = 0; i < 15; i++) {
        middle = (lower + upper) / 2.0f;
        int lumino = int(LuminosityFunc(middle));

        if (lumino < value) 
            lower = middle;
        else if (lumino > value) 
            upper = middle;
        else 
            break;
    }

    lightColor[index] = middle;
    UpdateOutput();
    return GetOutput(index);
}
