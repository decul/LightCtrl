#include <IRremote.h>
#include "hsv2rgbw.h"
#include <Arduino.h>


const int ledPins[4] = { 10, 9, 6, 5 };
int ledValues[4] = { 5, 0, 0, 0 };
int ledWeights[4] = { 255, 108, 56, 56 };
double rgbwValues[4] = { 0.5, 0.0, 0.0, 0.0 };
double hsvValues[3] = { 0.0, 1.0, 1.0 };

bool normalized = false;
bool lampOn = true;


IRrecv irrecv(7);
decode_results irResult;
long previousCode = 0;


int strobePeriod = 50;
long strobeTimer;
bool strobeEnabled = false;


void updateOutput();



void setup() {
    Serial.begin(9600);
    irrecv.enableIRIn();
	for (int l = 0; l < 4; l++)
		pinMode(ledPins[l], OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    updateOutput();

    //test(997);
}



void loop() {
    if (irrecv.decode(&irResult)) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        handleIrCode(irResult.value);
        //Serial.println(irResult.value, HEX);
        irrecv.resume(); // Receive the next value
    }

    if (strobeEnabled && millis() >= strobeTimer) {
        digitalWrite(ledPins[3], HIGH);
        delay(1);
        digitalWrite(ledPins[3], LOW);
        strobeTimer += strobePeriod;
    }
}



void handleIrCode(long code) {
    int color;
    bool sign = 0;
    int mode = 0;

    if (code == 0xFFFFFFFF)
        code = previousCode;
    previousCode = code;
    
    switch (code) {
        case 0xFFB04F:  // Red -
            sign = 1;
        case 0xFF906F:  // Red +
            color = 0;
            mode = 1;
            break;

        case 0xFF30CF:  // Green -
            sign = 1;
        case 0xFF10EF:  // Green +
            color = 1;
            mode = 1;
            break;

        case 0xFF708F:  // Blue -
            sign = 1;
        case 0xFF50AF:  // Blue +
            color = 2;
            mode = 1;
            break;

        case 0xFFF00F:  // White -
            sign = 1;
        case 0xFFD02F:  // White +
            color = 3;
            mode = 1;
            break;


        
        case 0xFFA857:  // H -
            sign = 1;
        case 0xFF28D7:  // H +
            color = 0;
            mode = 2;
            break;

        case 0xFF9867:  // S -
            sign = 1;
        case 0xFF18E7:  // S +
            color = 1;
            mode = 2;
            break;

        case 0xFF8877:  // V -
            sign = 1;
        case 0xFF08F7:  // V +
            color = 2;
            mode = 2;
            break;



        case 0xFF609F:  // OFF
            lampOn = false;
            break;

        case 0xFFE01F:  // ON
            lampOn = true;
            break;



        case 0xFFA05F:  // UP
            strobePeriod *= 1.11;
            strobePeriod += 1;
            Serial.println(strobePeriod);
            break;

        case 0xFF20DF:  // DOWN
            strobePeriod *= 0.9;
            Serial.println(strobePeriod);
            break;

    }

    switch (mode) {
        case 1:
            if (sign) {
                ledValues[color] *= 0.9;
            }
            else {
                ledValues[color] *= 1.11;
                ledValues[color] += 1;
                if (ledValues[color] > 255)
                    ledValues[color] = 255;
            }
            break;

        case 2:
            if (sign) {
                
            }
            else {
                
            }
    }
    
    updateOutput();
}



//void serialEvent() {
////    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
//
//    char c = Serial.read();
//    Serial.write(c);
//
//    delay(10);
//
//    switch (c) {
//        case 'r':
//            char rgbw[4];
//            for (int l = 0; l < 4; l++) {
//                if (Serial.available())
//                    values[l] = map(Serial.read(), '0', '9', 0, 255);
//                else
//                    values[l] = 0;
//                Serial.write(map(values[l], 0, 255, '0', '9'));
//            }
//            Serial.write('\n');
//            updateOutput();
//            break;
//    }
//
//    while (Serial.available())
//        Serial.read();
//}



void updateOutput() {
    for (int l = 0; l < 4; l++) {
        analogWrite(ledPins[l], lampOn ? ledValues[l] : LOW);
        Serial.print(ledValues[l]);
        Serial.print(" ");
    }
    Serial.println();
}
