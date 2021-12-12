#pragma once
#include <MillisTime.hpp>
#include "StrobeLight.hpp"

class SmoothLight : public StrobeLight {
private:
    
    float targetColor[COLOR_COUNT];
    float targetBrightness;
    bool transitionInProgres = false;
    MillisTimer transitionTimer;

public:
    float transitionSpeed = 0.01;


    void ColorTransition(String* rgbwy, String brightness) {
        ColorTransition(
            rgbwy[0].toFloat(),
            rgbwy[1].toFloat(),
            rgbwy[2].toFloat(),
            rgbwy[3].toFloat(),
            rgbwy[4].toFloat(),
            brightness.toFloat()
        );
    }

    void ColorTransition(float r, float g, float b, float w, float y, float brightness) {
        targetColor[0] = Limit(r);
        targetColor[1] = Limit(g);
        targetColor[2] = Limit(b);
        targetColor[3] = Limit(w);
        targetColor[4] = Limit(y);
        targetBrightness = Limit(brightness);

        transitionInProgres = true;
        transitionTimer.Start(10);
    }

    void HandleColorTransition() {
        if (transitionInProgres && transitionTimer.HasExpired()) {
            float maxDelta = targetBrightness - brightness;
            maxDelta = fabs(maxDelta);
            for (byte i = 0; i < COLOR_COUNT; i++) {
                float delta = targetColor[i] - lightColor[i];
                delta = fabs(delta);
                maxDelta = max(delta, maxDelta);
            }
            
            if (maxDelta == 0.0) {
                transitionInProgres = false;
                if (transitionSpeed < 0.01)
                    transitionSpeed = 0.01;
                return;
            }

            float newColor[COLOR_COUNT];
            for (byte i = 0; i < COLOR_COUNT; i++) {
                float delta = targetColor[i] - lightColor[i];
                float change = delta / maxDelta * transitionSpeed;
                if (fabs(change) >= fabs(delta)) 
                    newColor[i] = targetColor[i];
                else 
                    newColor[i] = lightColor[i] + change;
            }
            
            float deltaB = targetBrightness - brightness;
            float changeB = deltaB / maxDelta * transitionSpeed;
            float newBrightness = brightness + changeB;
            if (fabs(changeB) >= fabs(deltaB)) 
                newBrightness = targetBrightness;

            SetColors(newColor, newBrightness);

            transitionTimer.Continue();
        }
    }
    
    String GetTargetColors() {
        String str = "";
        for (byte l = 0; l < COLOR_COUNT; l++) 
            str += String(targetColor[l], 3) + " ";
        return str + String(targetBrightness, 3);
    }

};