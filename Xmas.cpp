// #include "Xmas.h"
// #include "Light.h"
// #include <Arduino.h>

// Light xlight;


// void Xmas::Enable() {
//     if (++mode > 2)
//         mode = 1;
// }

// void Xmas::Disable() {
//     mode = 0;
// }

// void Xmas::Run() {
//     if (mode != 0 && millis() >= timer) {
//         double* rgbw;

//         switch (mode) {
//             case 1:
//                 if (++state > 2) {
//                     state = 1;
//                 }
//                 switch (state) {
//                     case 1:     rgbw = new double[4]{0.0, 0.0, 0.0, 1.0};      break;
//                     case 2:     rgbw = new double[4]{0.0, 0.0, 1.0, 0.0};      break;
//                 }
//                 timer += 800;
//                 break;

//             case 2:
//                 if (++state > 6) {
//                     state = 1;
//                 }
//                 switch (state) {
//                     case 1:     rgbw = new double[4]{0.0, 0.0, 1.0, 0.0};      break;
//                     case 2:     rgbw = new double[4]{0.0, 1.0, 1.0, 0.0};      break;
//                     case 3:     rgbw = new double[4]{0.0, 1.0, 0.0, 0.0};      break;
//                     case 4:     rgbw = new double[4]{1.0, 0.4, 0.0, 0.0};      break;
//                     case 5:     rgbw = new double[4]{1.0, 0.0, 0.0, 0.0};      break;
//                     case 6:     rgbw = new double[4]{1.0, 0.0, 0.5, 0.0};      break;
//                 }
//                 timer += 500;
//                 break;
//         }

//         xlight.SetColor(rgbw);
//         delete[] rgbw;
//     }

//     if (timer > millis() + 100000)
//         timer = millis() + 1000; 
// }