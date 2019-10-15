#pragma once
#include "Timer.h"
#include "Light.h"

#define COL_NO 4

class Xmas {
private:
    int mode = 0;
    // int state = 0;
    // MillisTimer timer;

    Light *light;
    float prevColor[COL_NO];

public:
    Xmas(Light *light) {
        this->light = light;
    }

    void Enable() {
        mode = 1;
        for (int i = 0; i < COL_NO; i++) {
            prevColor[i] = light->GetColor(i);
        }
        //timer.Start(0);     // Just setting start time, needed for AddTime()
    }

    void Disable() {
        mode = 0;
        light->SetColor(prevColor, COL_NO);
    }

    void Switch() {
        if (!mode)
            Enable();
        else 
            Disable();
    }


    void Run() {
        if (mode == 1) {
            long time = millis();
            float rgb[3];
            
            for (int i = 0; i < 3; i++)
                rgb[i] = Level(i, time);

            light->SetColor(rgb, 3);
        }

        // else if (mode == 2 && timer.HasExpired()) {
        //     Serial.println(state);
        //     if (state % 2 == 0) {
        //         light->SetColor(sequence[state / 2], COL_NO);
        //         timer.AddTime(1000 - int(sequence[state / 2][COL_NO]));
        //         state++;
        //     }
        //     else {
        //         float zero[] = {0.0, 0.0, 0.0, 0.0};
        //         light->SetColor(zero, COL_NO);
        //         timer.AddTime(sequence[state / 2][COL_NO]);
        //         if (++state >= SEQ_LEN * 2)
        //             state = 0;
        //     }
        // }
    }

    float Level(int pin, long time) {
        long period = 10000;
        long offset = period / 3;

        float t = float((time + pin * offset) % period) / period;
        return max(0.08, min(1.0, min(6 * t, 4 - 6 * t)));
    }
};