#pragma once

#define OutPos 0
#define IPos 1
#define PPos 2

#define COLOR_COUNT 5
#define POINT_COUNT 2

class LedCharacteristics {
private:
    float points[COLOR_COUNT][POINT_COUNT][3] = {
        {
            { 25, 0.0, 0.0 },
            { 187, 1.0, 1.0 }
        },
        {
            { 23, 0.0, 0.0 },
            { 176, 1.0, 1.0 }
        },
        {
            { 24, 0.0, 0.0 },
            { 198, 1.0, 1.0 }
        },
        {
            { 25, 0.0, 0.0 },
            { 225, 1.0, 1.0 }
        },
        {
            { 50, 0.0, 0.0 },
            { 255, 1.0, 1.0 }
        }
    };

    float x2y(float xVal, byte color, byte xPos, byte yPos) {
        for (int i = 0; i < POINT_COUNT; i++) {
            if (points[color][i][xPos] == xVal) {
                return points[color][i][yPos];
            }
            if (points[color][i][xPos] > xVal) {
                if (i == 0) {
                    return points[color][0][yPos];
                }

                float Dx = points[color][i][xPos] - points[color][i - 1][xPos];
                float dx = xVal - points[color][i - 1][xPos];
                float ratio = dx / Dx;

                float Dy = points[color][i][yPos] - points[color][i - 1][yPos];
                float dy = ratio * Dy;

                float yVal = points[color][i - 1][yPos] + dy;
                return yVal;
            }
        }

        return points[color][COLOR_COUNT][yPos];
    }

public:
    // byte I2Out(float I, byte color) {
    //     return x2y(I, color, IPos, OutPos) + 0.5;
    // }

    // float Out2I(byte output, byte color) {
    //     return x2y(output, color, OutPos, IPos);
    // }

    // byte P2Out(float P, byte color) {
    //     return x2y(P, color, PPos, OutPos) + 0.5;
    // }

    // float Out2P(byte output, byte color) {
    //     return x2y(output, color, OutPos, PPos);
    // }

    byte Perc2Out(float perc, byte color) {
        return x2y(perc, color, IPos, OutPos) + 0.5;
    }

    float Out2Perc(byte output, byte color) {
        return x2y(output, color, OutPos, IPos);
    }
};