#include "Light.h"
#include <math.h>

Light::Light() {
	for (int l = 0; l < COLOR_COUNT; l++) 
        pinMode(ledPins[l], OUTPUT);
    Power(memory.IsOnByDefault());
}

void Light::UpdateOutput() {
    for (int l = 0; l < COLOR_COUNT; l++) 
        analogWrite(ledPins[l], GetOutput(l));
}


void Light::AdjustColor(int index, double difference) {
    Power(true);
    lightColor[index] += max(0.0, min(1.0, lightColor[index] + difference));
    UpdateOutput();
    UpdateDimmer();
}


void Light::SetColor(int index, float value) {
    lightColor[index] = max(0.0f, min(1.0f, value));
    UpdateOutput();
    UpdateDimmer();
}

void Light::SetColors(float* rgbwy, int count) {
    for (int l = 0; l < count; l++)
        lightColor[l] = max(0.0f, min(1.0f, rgbwy[l]));
    UpdateOutput();
    UpdateDimmer();
}

void Light::SetColors(String* rgbwy) {
    for (int l = 0; l < COLOR_COUNT; l++)
        lightColor[l] = max(0.0f, min(1.0f, rgbwy[l].toFloat()));
    UpdateOutput();
    UpdateDimmer();
}

float Light::GetColor(int index) {
    return lightColor[index];
}

String Light::GetColors() {
    String str = "";
    for (int l = 0; l < COLOR_COUNT; l++) 
        str += String(lightColor[l]) + " ";
    return str;
}


void Light::Power(bool on) {
    if (powerOn != on) {
        powerOn = on;
        UpdateOutput();
        memory.SetOnByDefault(on);
    }
}
    
void Light::Switch() {
    Power(!powerOn);
}

byte Light::GetOutput(int l) {
    if (!powerOn || lightColor[l] == 0.0) 
        return 0;
    else 
        return pow(255, lightColor[l]);
}

String Light::GetOutputs() {
    String str = "";
    for (int l = 0; l < COLOR_COUNT; l++) 
        str += GetOutput(l) + " ";
    return str;
}


void Light::StartStrobe(float width, float frequency) {
    Power(false);
    strobeEnabled = true;
    float strobePeriod = 1000000.0 / frequency;
    strobePeriodTimer.Start(strobePeriod);
    strobeDurationTimer.SetPeriod(width * strobePeriod);
}

void Light::HandleStrobe() {
    if (strobeEnabled) {
        Power(false);
        strobePeriodTimer.WaitForExpiration();
        strobeDurationTimer.Restart();
        digitalWrite(ledPins[4], HIGH);
        strobeDurationTimer.WaitForExpiration();
        digitalWrite(ledPins[4], LOW);
        strobePeriodTimer.Continue();
    }
}

void Light::StopStrobe() {
    Power(true);
    strobeEnabled = false;
}

#define defDimmerSpan 2
#define defDimmerResetHour 10
#define dimmerMinPeriod 30

void Light::HandleAutoDimming() {
    DateTime now = DateTime::Now();
    
    // Reset dimmer once a day
    if (now > dimmerResetTime && !powerOn) {
        ResetDimmer();
    }

    // Dim light if that's right time
    if (!dimmerDisabled && !dimmerFinished && now > dimmerStartTime) {
        TimeSpan dimmerSpan = dimmerEndTime - dimmerStartTime;
        TimeSpan timeRemaining = dimmerEndTime - now;
        float percent = max(0.0f, (float)timeRemaining.TotalSeconds() / dimmerSpan.TotalSeconds());

        for (int i = 2; i < COLOR_COUNT; i++) 
            lightColor[i] = percent * dimmerInitColor[i];
        UpdateOutput();

        if (now > dimmerEndTime) 
            dimmerFinished = true;
    }
}

void Light::ResetDimmer() {
    for (int i = 0; i < COLOR_COUNT; i++) {
        lightColor[i] = memory.GetDefaultColor(i);
        dimmerInitColor[i] = lightColor[i];
    }
    UpdateOutput();

    dimmerDisabled = false;
    dimmerFinished = false;
    DateTime now = DateTime::Now();

    Time dimEnd = memory.GetDefaultDimEndTime();
    dimmerEndTime = DateTime::ClosestDate(now, dimEnd);

    if (now.time() > dimEnd || now.Hour() < defDimmerResetHour)
        dimmerEndTime = now;

    dimmerStartTime = dimmerEndTime - TimeSpan(0, defDimmerSpan, 0, 0);
    dimmerResetTime = DateTime::ClosestDate(now, Time(defDimmerResetHour));
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

    DateTime now = DateTime::Now();
    if (now > dimmerStartTime) {
        dimmerStartTime = now;

        TimeSpan minPeriod = TimeSpan(0, 0, dimmerMinPeriod, 0);

        if (dimmerEndTime - now < minPeriod) {
            dimmerEndTime = now + minPeriod;
        }
    }
}

void Light::SetColorAsDefault() {
    for (int i = 0; i < COLOR_COUNT; i++) 
        memory.SetDefaultColor(i, lightColor[i]);
}



void Light::Flash(long us) {
    digitalWrite(ledPins[4], 1);
    MicrosTimer::Sleep(us);
    digitalWrite(ledPins[4], 0);
}