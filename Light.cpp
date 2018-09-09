#include "Light.h"
#include <USBAPI.h>
#include "Converter.h"
#include <math.h>

Light::Light() {
	for (int l = 0; l < 4; l++)
        pinMode(ledPins[l], OUTPUT);
    UpdateOutput();
}

void Light::UpdateOutput() {
    for (int l = 0; l < 4; l++) {
        if (rgbwValues[l] == 0.0)
            digitalWrite(ledPins[l], LOW);
        else if (rgbwValues[l] == 1.0)
            digitalWrite(ledPins[l], powerOn ? HIGH : LOW);
        else {
            int value = rgbwValues[l] * (ledMax[l] - ledMin[l]) + ledMin[l];
            analogWrite(ledPins[l], powerOn ? value : LOW);
        }
        //Serial.print(rgbwValues[l]);
        //Serial.print(" ");
    }
    //Serial.println();
}

void Light::IncreaseRgb(int color) {
    double val = sqrt(rgbwValues[color]) + 0.02;
    rgbwValues[color] = min(1.0, val * val + 0.01);
    // if (rgbwValues[color] < 0.01)
    //     rgbwValues[color] = 0.01;

    // rgbwValues[color] *= 1.05;
    if (rgbwValues[color] > 1.0)
        rgbwValues[color] = 1.0;
    UpdateOutput();
}

void Light::DecreaseRgb(int color) {
    double val = max(0.0, sqrt(rgbwValues[color]) - 0.02);
    rgbwValues[color] = max(0.0, val * val - 0.01);
    //rgbwValues[color] /= 1.05;
    UpdateOutput();
}

void Light::SwitchRgb(int color) {
    if (rgbwValues[color] > 0.0)
        rgbwValues[color] = 0.0;
    else
        rgbwValues[color] = 1.0;
    UpdateOutput();
}

void Light::SetRgbw(double* rgbw) {
    for (int i = 0; i < 4; i++)
        rgbwValues[i] = rgbw[i];
    UpdateOutput();
}

void Light::ChangeHue(int sign) {
    HSV hsv = Converter::rgbw2hsv({rgbwValues[0], rgbwValues[1], rgbwValues[2], 0});
    hsv.h += 2 * sign ? -1 : 1;
    RGBW col = Converter::hsv2rgbw(hsv);
    rgbwValues[0] = col.r;
    rgbwValues[1] = col.g;
    rgbwValues[2] = col.b;
    UpdateOutput();
}

void Light::ChangeValue(int sign) {
    if (!sign) {
        for (int i = 0; i < 4; i++) {
            if (rgbwValues[i] > 0.97)
                return;
        }
        for (int i = 0; i < 4; i++) 
            rgbwValues[i] = sqrt(pow(rgbwValues[i], 2) * 1.2);
    }
    else {
        for (int i = 0; i < 4; i++)
            rgbwValues[i] = sqrt(pow(rgbwValues[i], 2) / 1.2);
    }
    UpdateOutput();
}

void Light::Power(bool on) {
    powerOn = on;
    UpdateOutput();
}