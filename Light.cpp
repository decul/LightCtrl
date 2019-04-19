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
        analogWrite(ledPins[l], GetOutput(l));
    }
    digitalWrite(binaryPin, (powerOn && lightColor[4] == 1.0));

    //Serial.println(String("\t\t") + GetColor());
}


void Light::Brighten() {

}

void Light::Brighten(int index) {
    Power(true);
    lightColor[index] += 0.04;
    if (lightColor[index] > 1.0)
        lightColor[index] = 1.0;
    UpdateOutput();
}

void Light::Darken() {

}

void Light::Darken(int index) {
    Power(true);
    lightColor[index] -= 0.04;
    if (lightColor[index] < 0.0)
        lightColor[index] = 0.0;
    UpdateOutput();
}


void Light::Switch(int index) {
    Power(true);
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
    if (powerOn != on) {
        powerOn = on;
        UpdateOutput();
    }
}
    
void Light::SwitchPower() {
    powerOn = !powerOn;
    UpdateOutput();
}

void Light::SetOutput(int index, byte value) {
    lightColor[index] = characteristics.Out2Perc(value, index);
    UpdateOutput();
}

byte Light::GetOutput(int l) {
    if (!powerOn || lightColor[l] == 0.0) 
        return 0;
    else if (lightColor[l] == 1.0) 
        return 255;
    else 
        return characteristics.Perc2Out(lightColor[l], l);
}

String Light::GetOutput() {
    String str = "";
    for (int l = 0; l < COLOR_COUNT; l++) {
        str += GetOutput(l);
        str += " ";
    }
    return str;
}


void Light::StartStrobe(float width, float frequency) {
    Power(false);
    strobeEnabled = true;
    strobePeriod = 1000000.0 / frequency;
    strobeDuration = width * strobePeriod;
    nextStrobeTime = micros() + strobePeriod;
}

void Light::HandleStrobe() {
    if (strobeEnabled) {
        Power(false);

        long currentTime = micros();
        if (currentTime < prevStrobeTime)
            nextStrobeTime = 0;
        prevStrobeTime = currentTime;

        if (currentTime >= nextStrobeTime) {
            //strobeDurationTimer.Start(strobeDuration);
            digitalWrite(binaryPin, HIGH);
            //strobeDurationTimer.WaitForExpiration();
            delayMicroseconds(strobeDuration);
            digitalWrite(binaryPin, LOW);
            nextStrobeTime += strobePeriod;
        }
    }
}

void Light::StopStrobe() {
    Power(true);
    strobeEnabled = false;
}