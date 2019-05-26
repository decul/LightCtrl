#include "Light.h"
#include <USBAPI.h>
#include <math.h>

Light::Light() {
	for (int l = 0; l < COLOR_COUNT; l++) 
        pinMode(ledPins[l], OUTPUT);
    pinMode(binaryPin, OUTPUT);
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
    UpdateDimmer();
}

void Light::Darken() {

}

void Light::Darken(int index) {
    Power(true);
    lightColor[index] -= 0.04;
    if (lightColor[index] < 0.0)
        lightColor[index] = 0.0;
    UpdateOutput();
    UpdateDimmer();
}


void Light::Switch(int index) {
    Power(true);
    if (lightColor[index] > 0.0)
        lightColor[index] = 0.0;
    else
        lightColor[index] = 1.0;
    UpdateOutput();
    UpdateDimmer();
}


void Light::SetColor(int index, float value) {
    if (value > 1.0)
        value = (value - 2.0) / 100.0;
    lightColor[index] = value;
    UpdateOutput();
    UpdateDimmer();
}

void Light::SetColor(float* rgbw) {
    for (int i = 0; i < COLOR_COUNT; i++)
        lightColor[i] = rgbw[i];
    UpdateOutput();
    UpdateDimmer();
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
    UpdateDimmer();
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

#define defDimmerEndHour 23
#define defDimmerSpan 2
#define defDimmerResetHour 10
#define dimmerMinPeriod 30

void Light::HandleAutoDimming() {
    DateTime now = rtc.now();
    
    // Reset dimmer once a day
    if (now > dimmerResetTime && !powerOn) {
        ResetDimmer();
    }

    // Dim light if that's right time
    if (!dimmerDisabled && !dimmerFinished && now > dimmerStartTime) {
        TimeSpan dimmerSpan = dimmerEndTime - dimmerStartTime;
        TimeSpan timeRemaining = dimmerEndTime - now;
        float percent = max(0.0, (float)timeRemaining.totalseconds() / dimmerSpan.totalseconds());

        for (int i = 2; i < COLOR_COUNT; i++) 
            lightColor[i] = percent * dimmerInitColor[i];
        UpdateOutput();

        if (now > dimmerEndTime) 
            dimmerFinished = true;
            
    }
}

void Light::ResetDimmer() {
    for (int i = 0; i < COLOR_COUNT; i++) {
        lightColor[i] = defaultColor[i];
        dimmerInitColor[i] = defaultColor[i];
    }
    UpdateOutput();

    dimmerDisabled = false;
    dimmerFinished = false;
    DateTime now = rtc.now();

    dimmerEndTime = DateTime::ClosestDate(now, defDimmerEndHour, 0);
    dimmerStartTime = dimmerEndTime - TimeSpan(0, defDimmerSpan, 0, 0);
    dimmerResetTime = DateTime::ClosestDate(now, defDimmerResetHour, 0);
}

void Light::EnableDimmer() {
    dimmerDisabled = false;
    UpdateDimmer();
}

void Light::DisableDimmer() {
    dimmerDisabled = true;
}

void Light::UpdateDimmer() {
    if (lightColor[2] == 0.0 && lightColor[3] == 0.0 && lightColor[4] == 0.0) {
        dimmerFinished = true;
        return;
    }

    for (int i = 0; i < COLOR_COUNT; i++) 
        dimmerInitColor[i] = lightColor[i];
    dimmerFinished = false;

    DateTime now = rtc.now();
    if (now > dimmerStartTime) {
        dimmerStartTime = now;

        TimeSpan minPeriod = TimeSpan(0, 0, dimmerMinPeriod, 0);

        if (dimmerEndTime - now < minPeriod) {
            dimmerEndTime = now + minPeriod;
        }
    }
}