#pragma once

#define OutPos 0
#define PPos 1

#define COLOR_COUNT 5
#define POINT_COUNT 19

class LedCharacteristics {
private:
    float points[COLOR_COUNT][POINT_COUNT][2] = {
        {
            { 0,    0.0    },
            { 30,   0.0081 },
            { 40,   0.0796 },
            { 50,   0.1872 },
            { 60,   0.3149 },
            { 80,   0.6154 },
            { 100,  1.0172 },
            { 110,  1.2417 },
            { 120,  1.4454 },
            { 130,  1.6780 },
            { 140,  1.894  },
            { 160,  2.4408 },
            { 180,  2.9322 },
            { 200,  3.5296 },
            { 210,  3.6223 },
            { 220,  3.6993 },
            { 230,  3.7408 },
            { 250,  3.7504 },
            { 255,  3.7536 }
        },
        {
            { 0,    0.0    },
            { 30,   0.0086 },
            { 40,   0.0732 },
            { 50,   0.1670 },
            { 60,   0.2730 },
            { 70,   0.3883 },
            { 90,   0.6417 },
            { 110,  0.9157 },
            { 120,  1.0637 },
            { 130,  1.2029 },
            { 140,  1.3782 },
            { 160,  1.6864 },
            { 170,  1.8497 },
            { 180,  2.0387 },
            { 190,  2.178  },
            { 210,  2.5714 },
            { 220,  2.7192 },
            { 250,  3.0342 },
            { 255,  3.1563 }
        },
        {
            { 0,    0.0    },
            { 30,   0.0149 },
            { 40,   0.1535 },
            { 50,   0.3519 },
            { 60,   0.5634 },
            { 80,   1.0545 },
            { 100,  1.5740 },
            { 110,  1.8526 },
            { 130,  2.3826 },
            { 140,  2.6544 },
            { 150,  2.9198 },
            { 170,  3.2004 },
            { 190,  3.3495 },
            { 210,  3.495  },
            { 220,  3.507  },
            { 230,  3.516  },
            { 240,  3.522  },
            { 250,  3.5837 },
            { 255,  3.6456 }
        },
        {
            {   0,  0.0    },
            {  30,	0.0228 },
            {  40,	0.2740 },
            {  50,	0.6528 },
            {  60,	1.0825 },
            {  70,	1.5448 },
            {  80,	2.0424 },
            { 100,	3.0928 },
            { 120,	4.1574 },
            { 130,	4.6395 },
            { 140,	4.8997 },
            { 150,	5.1548 },
            { 160,	5.3252 },
            { 170,	5.5354 },
            { 180,	5.6763 },
            { 190,	5.8453 },
            { 210,	6.1359 },
            { 230,	6.3809 },
            { 255,	6.6761 }
        },
        {
            {   0,  0.0    },
            {  60,	0.0933 },
            {  70,	0.4411 },
            {  80,	0.9888 },
            {  90,	1.6036 },
            { 100,	2.3301 },
            { 110,	3.1081 },
            { 120,	3.971  },
            { 130,	4.8536 },
            { 140,	5.8092 },
            { 150,	6.8280 },
            { 160,	7.816  },
            { 170,	8.8555 },
            { 180,	9.867  },
            { 190, 10.834  },
            { 200, 11.44   },
            { 210, 11.823  },
            { 220, 12.013  },
            { 255, 12.515  }
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

        return points[color][POINT_COUNT - 1][yPos];
    }

    float perc2P (float perc, byte color) {
        float Pmin = points[color][1][PPos];
        float Pmax = points[color][POINT_COUNT - 1][PPos];
        float D = Pmax / Pmin;
        return Pmin * pow(D, perc);
    }

public:

    byte Perc2Out(float perc, byte color) {
        float P = perc2P(perc, color);
        return x2y(P, color, PPos, OutPos) + 0.5;
    }

    float Out2Perc(byte output, byte color) {
        return 0.0;
    }
};