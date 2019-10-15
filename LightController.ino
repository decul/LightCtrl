#include <IRremote.h>
#include "Xmas.h"
#include "Light.h"
#include "Button.h"
#include <Arduino.h>
#include "RTClib.h"
#include "MyEEPROM.h"
//#include <OneWire.h>

RTC_DS1307 rtc;
MyEEPROM memory;

// Uncomment #define IR_USE_TIMER5 in ...\IRremote\boarddefs.h to use timer from pin 46
IRrecv irrecv(46);

long prevIRCode = 0;


Light light;
Xmas xmas(&light);
Button button(A10);
HardwareSerial* serials[4];


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
    
    light.ResetDimmer();
}



void loop() {
    decode_results irResult;
    if (irrecv.decode(&irResult)) {
        //digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        handleIrCode(irResult.value);
        irrecv.resume(); // Receive the next value
    }

    light.HandleStrobe();
    light.HandleAutoDimming();

    switch (button.GetAction()) {
        case Button::SWITCH_POWER:
            light.SwitchPower();
            break;

        case Button::BRIGHTEN:
            light.Brighten(4);
            break;

        case Button::DARKEN:
            light.Darken(4);
            break;
    }

    xmas.Run();
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
    const byte MAX_ARGS_LEN = 8;
    String args[MAX_ARGS_LEN];
    byte argsNo = 0;

    String command = getWord(input);
    
    while (input.length() > 0) {
        if (++argsNo >= MAX_ARGS_LEN) {
            return "Too many arguments given";
        }
        args[argsNo - 1] = getWord(input);
    }

    if (command == "time") {
        if (argsNo == 0)
            return rtc.now().toISO();
        else if (DateTime::FromISO(args[0]).unixtime() != 0)
            rtc.adjust(DateTime::FromISO(args[0]));
    }

    else if (command == "ir") {
        char *p;
        handleIrCode(strtoul(args[0].c_str(), &p, 16));
    }

    else if (command == "on") {
        light.Power(true);
    }

    else if (command == "off") {
        light.Power(false);
    }

    else if (command == "color") {
        if (argsNo == COLOR_COUNT) {
            for (int i = 0; i < COLOR_COUNT; i++) {
                light.SetColor(i, args[i].toFloat());
            }
        }
        else if (argsNo == 2) {
            if (args[1] == "+")
                light.Brighten(args[0].toInt());
            else if (args[1] == "-")
                light.Darken(args[0].toInt());
            else if (args[1] == "switch")
                light.Switch(args[0].toInt());
            else
                light.SetColor(args[0].toInt(), args[1].toFloat());
            return light.GetColor();
        }
        else if (argsNo == 1)
            return String(light.GetColor(args[0].toInt()));
        else if (argsNo == 0)
            return light.GetColor();
        else 
            return "Wrong number of arguments";
    }

    else if (command == "output") {
        if (argsNo == 0) {
            return light.GetOutput();
        }
        else if (argsNo == 2) {
            light.SetOutput(args[0].toInt(), args[1].toInt());
        }
    }

    else if (command == "quick") {
        analogWrite(light.ledPins[args[0].toInt()], args[1].toInt());
        delay(5000);
        digitalWrite(light.ledPins[args[0].toInt()], LOW);
    }

    else if (command == "strobe") {
        if (argsNo == 0)
            light.StopStrobe();
        else if (argsNo == 2)
            light.StartStrobe(args[0].toFloat(), args[1].toFloat());
        else 
            return "Wrong number of arguments";
    }

    else if (command == "dimmer") {
        if (argsNo == 0)
            return "No arguments given";
        else if (args[0] == "on")
            light.EnableDimmer();
        else if (args[0] == "off")
            light.DisableDimmer();
        else if (args[0] == "setdefcol")
            light.SetColorAsDefault();
        else if (args[0] == "gettime")
            return memory.GetDefaultDimEndTime().toISO();
        else if (args[0] == "settime" && argsNo == 2)
            memory.SetDefaultDimEndTime(Time::FromISO(args[1]));
        else 
            return "Invalid arguments";
    }

    else if (command == "xmas") {
        if (argsNo == 0)
            xmas.Switch();
    }

    else if (command == "reset") {
        for (int i = 0; i < 2; i++)
            serials[i]->println("=== Software Reset ===");
        delay(50);
        reset();
    }

    else if (command == "?" || command == "help") {
        String man = "";

        man += "string time();\n";
        man += "void time(string isoTime);\n\n";

        man += "void ir(string hexCode);\n\n";

        man += "void on();\n";
        man += "void off();\n\n";

        man += "string color();\n";
        man += "float color(int led);\n";
        man += "string color(int led, float value);\n";
        man += "string color(int led, [+/-/switch]);\n";
        man += "string color(float col[5]);\n\n";

        man += "string output();\n";
        man += "void output(int led, int value);\n\n";

        man += "void strobe(float width, float freq);\n";
        man += "void strobe();\n\n";

        man += "void dimmer([on/off/setdefcol/gettime]);\n";
        man += "void dimmer([settime], string iso);\n\n";

        man += "void xmas();\n\n";

        man += "void reset();\n\n";

        return man;
    }

    else {
        return String("Unrecognized command: '") + command + "'";
    }

    return "OK";
}

String getWord(String &input) {
    String result;
    int spaceIndex = input.indexOf(' ');

    if (spaceIndex > -1) {
        result = input.substring(0, spaceIndex);
        input.remove(0, spaceIndex + 1);
    }
    else {
        result = input.substring(0, input.length());
        input.remove(0, input.length());
    }

    input.trim();
    result.trim();

    return result;
}