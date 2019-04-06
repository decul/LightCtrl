#include <IRremote.h>
#include "Xmas.h"
#include "Light.h"
#include <Arduino.h>
#include <RTClib.h>
//#include <OneWire.h>

RTC_DS1307 rtc;

// Uncomment #define IR_USE_TIMER5 in ...\IRremote\boarddefs.h to use timer from pin 46
IRrecv irrecv(46);

long prevIRCode = 0;


Light light;
HardwareSerial* serials[4];
// Xmas xmas;


//declare reset function @ address 0
void (*reset) (void) = 0; 



void setup() {
    serials[0] = &Serial;
    serials[1] = &Serial3;

    Serial.begin(115200);
    Serial3.begin(9600);
    irrecv.enableIRIn();
    pinMode(LED_BUILTIN, OUTPUT);
    
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
    }
    if (!rtc.isrunning()) {
        Serial.println("RTC is NOT running!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
}



void loop() {
    // xmas.Run();

    decode_results irResult;
    if (irrecv.decode(&irResult)) {
        //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        handleIrCode(irResult.value);
        irrecv.resume(); // Receive the next value
    }
}



void handleIrCode(long code) {
    Serial.println(code, HEX);

    switch (code) {
        case 0xFFA857:  light.Switch(0);     break;
        case 0xFF28D7:  light.Switch(1);     break;  
        case 0xFF6897:  light.Switch(2);     break;  
        case 0xFFE817:  light.Switch(3);     break;  
        
        default:

            if (code == 0xFFFFFFFF)
                code = prevIRCode;
            prevIRCode = code;
    
            switch (code) {
                case 0xFFB04F:  light.Darken(0);     break;
                case 0xFF906F:  light.Brighten(0);   break;
                case 0xFF30CF:  light.Darken(1);     break;
                case 0xFF10EF:  light.Brighten(1);   break;
                case 0xFF708F:  light.Darken(2);     break;
                case 0xFF50AF:  light.Brighten(2);   break;
                case 0xFFF00F:  light.Darken(3);     break;
                case 0xFFD02F:  light.Brighten(3);   break;
                case 0xFFA05F:  light.Brighten(4);   break;  // UP
                case 0xFF20DF:  light.Darken(4);     break;  // DOWN

                case 0xFF609F:  light.Power(false);     break;
                case 0xFFE01F:  light.Power(true);      break;
            }

            break;
    }

    if (code == 0xFFFFFFFF)
        code = prevIRCode;
    prevIRCode = code;
    
}



void serialEvent() { 
    handleSerialEvent(0);
}

void serialEvent3 () { 
    handleSerialEvent(1);
}

void handleSerialEvent(int s) {
    String command = "";

    while (serials[s]->available()) {
        char c = serials[s]->read();
        if (c == ';') 
            break;
        command += c;

        if (!serials[s]->available())
            delay(5);
    }

    command.trim();

    if (command.length() > 0) {
        command.toLowerCase();
        serials[s]->println(&handleCommand(command)[0]);
        command.remove(0);
    }
}

String handleCommand(String input) {
    const byte ARGS_LEN = 8;
    String args[ARGS_LEN];
    byte argIndex = 0;
    
    while (input.length() > 0) {
        if (argIndex > ARGS_LEN) {
            return "Too many arguments given";
        }
        int spaceIndex = input.indexOf(' ');
        if (spaceIndex > -1) {
            args[argIndex] = input.substring(0, spaceIndex);
            input.remove(0, spaceIndex + 1);
            input.trim();
            argIndex++;
        }
        else {
            args[argIndex] = input;
            break;
        }
    }

    if (args[0] == "time") {
        if (args[1].length() == 0)
            return (timeToISO(rtc.now()) + "\n").c_str();
        else if (timeFromISO(args[1]).unixtime() != 0)
            rtc.adjust(timeFromISO(args[1]));
    }

    else if (args[0] == "ir") {
        char *p;
        handleIrCode(strtoul(args[1].c_str(), &p, 16));
    }

    else if (args[0] == "on") {
        light.Power(true);
    }

    else if (args[0] == "off") {
        light.Power(false);
    }

    else if (args[0] == "color") {
        if (args[4].length() > 0) {
            for (int i = 0; i < 4; i++) {
                light.SetColor(i, args[i + 1].toFloat());
            }
        }
        else if (args[2].length() > 0) {
            if (args[2] == "+")
                light.Brighten(args[1].toInt());
            else if (args[2] == "-")
                light.Darken(args[1].toInt());
            else if (args[2] == "switch")
                light.Switch(args[1].toInt());
            else
                light.SetColor(args[1].toInt(), args[2].toFloat());
            return light.GetColor();
        }
        else if (args[1].length() > 0)
            return String(light.GetColor(args[1].toInt()));
        else 
            return light.GetColor();
    }

    else if (args[0] == "alpha") {
        if (args[1].length() > 0) {
            float alpha = args[1].toFloat();
            if (alpha > 1.0)
                alpha = (alpha - 2.0) / 100.0;
            light.alpha = alpha;
            light.UpdateOutput();
        }
        else
            return String(light.alpha);
    }

    else if (args[0] == "reset") {
        reset();
    }

    else if (args[0] == "?" || args[0] == "help") {
        String man = "";

        man += "string time();\n";
        man += "void time(string isoTime);\n\n";

        man += "void ir(string hexCode);\n\n";

        man += "void on();\n";
        man += "void off();\n\n";

        man += "string color();\n";
        man += "float color(int led);\n";
        man += "string color(int led, float value);\n";
        man += "string color(float r, float g, float b, float w);\n\n";

        man += "float alpha();\n";
        man += "void alpha(float value);\n\n";

        man += "void reset();\n\n";

        return man;
    }

    else {
        return String("Unrecognized command: '") + args[0] + "'";
    }

    return "OK";
}

String timeToISO(DateTime time) {
    char iso[20];
    sprintf(iso, "%04d-%02d-%02dT%02d:%02d:%02d", time.year(), time.month(), time.day(), time.hour(), time.minute(), time.second());
    return iso;
}

DateTime timeFromISO(String iso) {
    if (iso[4] != '-' || iso[7] != '-' || iso[10] != 't' || iso[13] != ':')
        return DateTime(0);
    
    return DateTime(
        iso.substring(0, 4).toInt(),
        iso.substring(5, 7).toInt(),
        iso.substring(8, 10).toInt(),
        iso.substring(11, 131).toInt(),
        iso.substring(14, 16).toInt(),
        (iso.length() == 19) ? iso.substring(17, 19).toInt() : 0
    );
}