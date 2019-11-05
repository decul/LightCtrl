#pragma once
#include <Arduino.h>
#include <IRremote.h>


class IrMsgr {
private:
    static IRrecv irrecv;
    static long prevIRCode;

    static String handleIrCode(long code) {
        Serial.println(code, HEX);

        switch (code) {
            case 0xFFA857:      return "color 0 switch";
            case 0xFF28D7:      return "color 1 switch";  
            case 0xFF6897:      return "color 2 switch";  
            case 0xFFE817:      return "color 3 switch";  
            
            default:

                if (code == 0xFFFFFFFF)
                    code = prevIRCode;
                prevIRCode = code;
        
                switch (code) {
                    case 0xFFB04F:      return "color 0 -";
                    case 0xFF30CF:      return "color 1 -";
                    case 0xFF708F:      return "color 2 -";
                    case 0xFFF00F:      return "color 3 -";
                    case 0xFF20DF:      return "color 4 -";
                    case 0xFF906F:      return "color 0 +";
                    case 0xFF10EF:      return "color 1 +";
                    case 0xFF50AF:      return "color 2 +";
                    case 0xFFD02F:      return "color 3 +";
                    case 0xFFA05F:      return "color 4 +";

                    case 0xFF609F:      return "off";
                    case 0xFFE01F:      return "on";

                    case 0xFFC837:      return "flash 1000";

                    default:            return String(code, 16);
                }

                break;
        }
    }

public:
    static void Initialize() {
        irrecv.enableIRIn();
    }

    static String GetCommand() {
        decode_results irResult;
        if (irrecv.decode(&irResult)) {
            String command = handleIrCode(irResult.value);
            irrecv.resume(); // Receive the next value
            return command;
        }
        else {
            return "";
        }
    }

};

// Uncomment #define IR_USE_TIMER5 in ...\IRremote\boarddefs.h to use timer from pin 46
IRrecv IrMsgr::irrecv(46);
long IrMsgr::prevIRCode = 0;