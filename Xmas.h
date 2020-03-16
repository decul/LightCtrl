#pragma once
#include "Timer.h"
#include "Light.h"

#define COL_NO 5
#define STATES_NO 5
#define TRANS_TIME 2000
#define STATE_TIME 2000

class Xmas {
private:
    int mode = 0;
    int state = 0;
    int prevState = STATES_NO - 1;
    int flashIndex = 1;
    byte flashOn = false;
    MillisTimer timer;

    long begin = 0;
    long transEnd = 0;



    Light *light;
    float colorBackup[5];

    float sequence[STATES_NO][5] = {
        //R    G     B      TRA  STA
        { 1.0, 0.0,  0.005, 1.0, 1.0 },     // Red
        { 1.0, 0.6,  0.0,   1.0, 1.0 },     // Yellow
        { 0.7, 1.0,  0.0,   1.0, 1.0 },     // Green
        { 0.0, 0.8,  1.0,   0.9, 1.0 },     // Blue
        { 0.8, 0.0,  1.0,   1.0, 1.0 },     // Purple
    };

    void Enable() {
        light->Power(true);
        for (int i = 0; i < 5; i++) {
            colorBackup[i] = light->GetColor(i);
        }
        flashOn = false;
    }

    void Disable() {
        light->SetColor(colorBackup, 5);
    }

public:
    Xmas(Light *light) {
        this->light = light;
    }

    void Switch() {
        if (!mode)
            Enable();

        if (++mode > 2)
            mode = 0;

        if (mode == 2)
            timer.Start(1000);

        if (!mode) 
            Disable();
    }


    void Run() {
        if (mode > 0) {
            if (mode == 2 && timer.HasExpired()) {
                flashOn = !flashOn;
                timer.Continue();
            }

            long now = millis();
            float rgb[COL_NO];

            long end = transEnd + sequence[state][4] * STATE_TIME;

            if (now > end || now < begin) {
                prevState = state;
                if (++state >= STATES_NO)
                    state = 0;
                begin = now;
                transEnd = now + sequence[state][3] * TRANS_TIME;
                Serial.println("State: " + String(state));
            }

            float perc = min(1.0, (float)(now - begin) / (transEnd - begin));

            for (int c = 0; c < 3; c++)
                rgb[c] = perc * sequence[state][c] + (1 - perc) * sequence[prevState][c];
            rgb[3] = 0.0;
            rgb[4] = flashOn * 0.5;

            light->SetColor(rgb, COL_NO);
        }
    }

    float Level(int pin, long time) {
        long period = 30000;
        long offset = period / 3;

        float t = float((time + pin * offset) % period) / period;
        return max(0.08, min(1.0, min(6 * t, 4 - 6 * t)));
    }
};