#pragma once

typedef struct {
    double r;       // a fraction between 0 and 1
    double g;       // a fraction between 0 and 1
    double b;       // a fraction between 0 and 1
    double w;       // a fraction between 0 and 1
} RGBW;

typedef struct {
    double h;       // angle in degrees
    double s;       // a fraction between 0 and 1
    double v;       // a fraction between 0 and 1
} HSV;

struct Converter {
    static HSV rgbw2hsv(RGBW in);
    static RGBW hsv2rgbw(HSV in);
};