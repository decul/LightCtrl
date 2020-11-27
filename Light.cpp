#include "Light.h"
#include <math.h>

#define PWM_RANGE 4095
#define LINEAR_COMPONENT 1694
#define FOURTH_COMPONENT 7


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
    UpdateDimmer();
}


void Light::SetColor(byte index, float value) {
    lightColor[index] = Limit(value);
    UpdateOutput();
    UpdateDimmer();
}

void Light::SetColors(float* rgbwy, byte count) {
    for (byte l = 0; l < count; l++)
        lightColor[l] = Limit(rgbwy[l]);
    UpdateOutput();
    UpdateDimmer();
}

void Light::SetColors(String* rgbwy, String brightness) {
    if (brightness != "")
        this->brightness = brightness.toFloat();
    for (byte l = 0; l < COLOR_COUNT; l++)
        lightColor[l] = Limit(rgbwy[l].toFloat());
    UpdateOutput();
    UpdateDimmer();
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


void Light::StartStrobe(float width, float frequency) {
    Power(false);
    strobeMode = 1;
    float strobePeriod = 1000000.0 / frequency;
    strobePeriodTimer.Start(strobePeriod);
    strobeDurationTimer.SetPeriod(width * strobePeriod);
}

void Light::StartRGBStrobe(float frequency) {
    Power(false);
    strobeMode = 2;
    float strobePeriod = 1000000.0 / frequency;
    strobePeriodTimer.Start(strobePeriod);
}

void Light::HandleStrobe() {
    if (strobeMode == 2) {
        Power(false);
        strobePeriodTimer.WaitForExpiration();
        //digitalWrite(ledPins[strobeColor], LOW);
        if (++strobeColor > 2)
            strobeColor = 0;
        //digitalWrite(ledPins[strobeColor], HIGH);
        strobePeriodTimer.Continue();
    }
    else if (strobeMode == 1) {
        Power(false);
        strobePeriodTimer.WaitForExpiration();
        strobeDurationTimer.Restart();
        //digitalWrite(ledPins[4], HIGH);
        strobeDurationTimer.WaitForExpiration();
        //digitalWrite(ledPins[4], LOW);
        strobePeriodTimer.Continue();
    }
}

void Light::StopStrobe() {
    Power(true);
    strobeMode = 0;
}

#define defDimmerSpan 2
#define defDimmerResetHour 10
#define dimmerMinPeriod 30

void Light::HandleAutoDimming() {
    if (DateTime::IsSet() && dimmerTimer.HasExpired()) {
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

            for (byte i = 2; i < COLOR_COUNT; i++) 
                lightColor[i] = percent * dimmerInitColor[i];
            UpdateOutput();

            if (now > dimmerEndTime) 
                dimmerFinished = true;
        }

        dimmerTimer.Start(1000);
    }
}

void Light::ResetDimmer() {
    brightness = 1.0f;
    for (byte i = 0; i < COLOR_COUNT; i++) {
        lightColor[i] = MyEEPROM::GetDefaultColor(i);
        dimmerInitColor[i] = lightColor[i];
    }
    UpdateOutput();

    dimmerDisabled = false;
    dimmerFinished = false;

    DateTime now = DateTime::Now();
    dimmerResetTime = now.ClosestDate(Time(defDimmerResetHour));

    Time dimEnd = MyEEPROM::GetDefaultDimEndTime();
    dimmerEndTime = now.ClosestDate(dimEnd);
    if (dimmerEndTime >= dimmerResetTime) 
        dimmerEndTime = now;

    dimmerStartTime = dimmerEndTime - TimeSpan(0, defDimmerSpan, 0, 0);
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

    for (byte i = 0; i < COLOR_COUNT; i++) 
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
    for (byte i = 0; i < COLOR_COUNT; i++) 
        MyEEPROM::SetDefaultColor(i, lightColor[i]);
}



void Light::Flash(long us) {
    //digitalWrite(ledPins[4], 1);
    MicrosTimer::Sleep(us);
    //digitalWrite(ledPins[4], 0);
}