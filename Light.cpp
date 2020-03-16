#include "Light.h"
#include <USBAPI.h>
#include <math.h>

Light::Light() {
	for (int l = 0; l < COLOR_COUNT; l++) 
        pinMode(ledPins[l], OUTPUT);
    pinMode(binaryPin, OUTPUT);
    Power(memory.IsOnByDefault());
}

void Light::UpdateOutput() {
    for (int l = 0; l < COLOR_COUNT; l++) {
        analogWrite(ledPins[l], GetOutput(l));
    }
    //digitalWrite(binaryPin, (powerOn && lightColor[4] == 1.0));
    analogWrite(binaryPin, GetOutput(5));
}


void Light::Adjust(int index, double value) {
    Power(true);
    lightColor[index] += value;
    if (lightColor[index] > 1.0)
        lightColor[index] = 1.0;
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

void Light::SetColor(float* rgbwy, int count) {
    memcpy(lightColor, rgbwy, sizeof(float) * count);
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
        memory.SetOnByDefault(on);
    }
}
    
void Light::SwitchPower() {
    Power(!powerOn);
}

byte Light::GetOutput(int _l) {
    int l = _l;
    if (!filterEnabled) {
        if (_l == 4)
            return 0;
        if (_l == 5)
            l = 4;
    }
    if (l == 5) {
        return (powerOn && lightColor[4] == 1.0) ? 255 : 0;
    }
    
    if (!powerOn || lightColor[l] == 0.0) 
        return 0;
    else if (lightColor[l] == 1.0) 
        return 255;
    else 
        return characteristics.Perc2Out(lightColor[l], _l);
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
    long strobePeriod = 1000000.0 / frequency;
    strobePeriodTimer.Start(strobePeriod);
    strobeDurationTimer.period = width * strobePeriod;
}

void Light::HandleStrobe() {
    if (strobeEnabled) {
        Power(false);
        strobePeriodTimer.WaitForExpiration();
        strobeDurationTimer.Restart();
        digitalWrite(binaryPin, HIGH);
        strobeDurationTimer.WaitForExpiration();
        digitalWrite(binaryPin, LOW);
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
        lightColor[i] = memory.GetDefaultColor(i);
        dimmerInitColor[i] = lightColor[i];
    }
    UpdateOutput();

    dimmerDisabled = false;
    dimmerFinished = false;
    DateTime now = rtc.now();

    Time dimEnd = memory.GetDefaultDimEndTime();
    dimmerEndTime = DateTime::ClosestDate(now, dimEnd);

    if (now.time() > dimEnd || now.hour() < defDimmerResetHour)
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

    DateTime now = rtc.now();
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
    MicrosTimer timer;
    digitalWrite(binaryPin, 1);
    timer.Start(us);
    timer.WaitForExpiration();
    digitalWrite(binaryPin, 0);
}