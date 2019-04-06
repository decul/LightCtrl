#include "Light.h"
#include <USBAPI.h>
#include <math.h>

Light::Light() {
	for (int l = 0; l < COLOR_COUNT; l++) 
        pinMode(ledPins[l], OUTPUT);
    pinMode(binaryPin, OUTPUT);
    UpdateOutput();
}

void Light::UpdateOutput() {
    for (int l = 0; l < COLOR_COUNT; l++) {
        if (!powerOn || lightColor[l] == 0.0) {
            digitalWrite(ledPins[l], LOW);
        }
        else if (lightColor[l] == 1.0) {
            digitalWrite(ledPins[l], HIGH);
        }
        else {
            float c = lightColor[l];
            float colorWithAlpha = alpha * c * c + (1.0 - alpha) * c;
            int ledValue = colorWithAlpha * (ledMax[l] - ledMin[l]) + ledMin[l];
            analogWrite(ledPins[l], ledValue);
        }

        if (powerOn && lightColor[4] == 1.0) {
            digitalWrite(binaryPin, HIGH);
        }
        else {
            digitalWrite(binaryPin, LOW);
        }
    }
    //Serial.println(String("\t\t") + GetColor());
}


void Light::Brighten() {

}

void Light::Brighten(int index) {
    lightColor[index] += 0.04;
    if (lightColor[index] > 1.0)
        lightColor[index] = 1.0;
    UpdateOutput();
}

void Light::Darken() {

}

void Light::Darken(int index) {
    lightColor[index] -= 0.04;
    if (lightColor[index] < 0.0)
        lightColor[index] = 0.0;
    UpdateOutput();
}


void Light::Switch(int index) {
    if (lightColor[index] > 0.0)
        lightColor[index] = 0.0;
    else
        lightColor[index] = 1.0;
    UpdateOutput();
}


void Light::SetColor(int index, float value) {
    if (value > 1.0)
        value = (value - 2.0) / 100.0;
    lightColor[index] = value;
    UpdateOutput();
}

void Light::SetColor(float* rgbw) {
    for (int i = 0; i < COLOR_COUNT; i++)
        lightColor[i] = rgbw[i];
    UpdateOutput();
}

float Light::GetColor(int index) {
    return lightColor[index];
}

String Light::GetColor() {
    String str = "";
    for (int l = 0; l < COLOR_COUNT; l++) {
        str += lightColor[l];
        str += " ";
    }
    return str;
}


void Light::Power(bool on) {
    powerOn = on;
    UpdateOutput();
}

