#include <IRremote.h>
#include "Xmas.h"
#include "Light.h"
#include <Arduino.h>


IRrecv irrecv(7);
decode_results irResult;
long previousCode = 0;


int strobePeriod = 50;
long strobeTimer;
bool strobeEnabled = false;


Light light;
Xmas xmas;



void setup() {
    Serial.begin(9600);
    irrecv.enableIRIn();
    pinMode(LED_BUILTIN, OUTPUT);
}



void loop() {
    // Serial.println(analogRead(A5));
    // delay(50);

    xmas.Run();

    if (irrecv.decode(&irResult)) {
        //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        handleIrCode(irResult.value);
        Serial.println(irResult.value, HEX);
        irrecv.resume(); // Receive the next value
    }
}



void handleIrCode(long code) {
    switch (code) {
        case 0xFFA857:  light.SwitchRgb(0);     break;
        case 0xFF28D7:  light.SwitchRgb(1);     break;  
        case 0xFF6897:  light.SwitchRgb(2);     break;  
        case 0xFFE817:  light.SwitchRgb(3);     break;  
        
        case 0xFFA05F:  xmas.Enable();          break;  // UP
        case 0xFF20DF:  xmas.Disable();
                        light.UpdateOutput();   break;  // DOWN


        default:

            if (code == 0xFFFFFFFF)
                code = previousCode;
            previousCode = code;
    
            switch (code) {
                case 0xFFB04F:  light.DecreaseRgb(0);   break;
                case 0xFF906F:  light.IncreaseRgb(0);   break;
                case 0xFF30CF:  light.DecreaseRgb(1);   break;
                case 0xFF10EF:  light.IncreaseRgb(1);   break;
                case 0xFF708F:  light.DecreaseRgb(2);   break;
                case 0xFF50AF:  light.IncreaseRgb(2);   break;
                case 0xFFF00F:  light.DecreaseRgb(3);   break;
                case 0xFFD02F:  light.IncreaseRgb(3);   break;

                case 0xFF9867:  light.ChangeHue(0);     break;
                case 0xFF8877:  light.ChangeHue(1);     break;
                case 0xFF18E7:  light.ChangeValue(0);   break;  
                case 0xFF08F7:  light.ChangeValue(1);   break; 
                // case 0xFF8877:      break;  
                // case 0xFF08F7:      break; 

                case 0xFF609F:  light.Power(false);     break;
                case 0xFFE01F:  light.Power(true);      break;


                // default:
                //     light.Power(false);
                //     delay(100);
                //     light.Power(true);
                //     break;
            }

            break;
    }

    if (code == 0xFFFFFFFF)
        code = previousCode;
    previousCode = code;
    
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


