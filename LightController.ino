#include "Light.h"
#include "Button.h"
#include <Arduino.h>
#include <MillisTime.h>
#include "MyEEPROM.h"
#include "SerialMsgr.h"

MyEEPROM memory;


Light light;
Button button(A0);


//declare reset function @ address 0
void (*reset) (void) = 0; 


void setup() {
    SerialMsgr::Initialize();
    light.ResetDimmer();
    
}



void loop() {
    light.HandleStrobe();
    light.HandleAutoDimming();

    switch (button.GetAction()) {
        case Button::CLICK:
            light.Switch();
            break;

        case Button::HOLD:
            light.AdjustColor(4, 0.004);
            break;

        case Button::CLICK_HOLD:
            light.AdjustColor(4, -0.004);
            break;
    }
}



String handleCommand(String input) {
    const byte MAX_ARGS_LEN = 8;
    String args[MAX_ARGS_LEN];
    byte argsNo = 0;

    String command = SerialMsgr::GetWord(input);

    while (input.length() > 0) {
        if (++argsNo >= MAX_ARGS_LEN) {
            return "Too many arguments given";
        }
        args[argsNo - 1] = SerialMsgr::GetWord(input);
    }


    if (command == "on") {
        light.Power(true);
    }

    else if (command == "off") {
        light.Power(false);
    }

    else if (command == "switch") {
        light.Switch();
    }

    else if (command == "color") {
        switch (argsNo) {
            case 0:     return light.GetColors();
            case 1:     return String(light.GetColor(args[0].toInt()));
            case 2:     light.SetColor(args[0].toInt(), args[1].toFloat());     break;
            case 5:     light.SetColors(args);                                  break;
            default:    return "Wrong number of arguments";
        } 
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
            return memory.GetDefaultDimEndTime().ToString();
        else if (args[0] == "settime" && argsNo == 2)
            memory.SetDefaultDimEndTime(Time::FromString(args[1]));
        else 
            return "Invalid arguments";
    }

    else if (command == "strobe") {
        if (argsNo == 0)
            light.StopStrobe();
        else if (argsNo == 2)
            light.StartStrobe(args[0].toFloat(), args[1].toFloat());
        else 
            return "Wrong number of arguments";
    }

    else if (command == "flash") {
        if (argsNo == 0)
            return "No arguments given";
        else 
            light.Flash(args[0].toInt());
    }

    else if (command == "reset") {
        for (int i = 0; i < 2; i++)
            Serial.println("=== Software Reset ===");
        delay(50);
        reset();
    }

    else if (command == "time") {
        if (argsNo == 0)
            return DateTime::Now().ToISO();
        else if (DateTime::FromISO(args[0]).UnixTime() != 0)
            DateTime::Adjust(DateTime::FromISO(args[0]));
    }

    else if (command == "?" || command == "help") {
        String man = "";

        man += "void on();\n";
        man += "void off();\n";
        man += "void switch();\n\n";

        man += "string color();\n";
        man += "float color(int led);\n";
        man += "void color(int led, float value);\n";
        man += "void color(float col[5]);\n\n";

        man += "void dimmer([on/off/setdefcol/gettime]);\n";
        man += "void dimmer([settime], string iso);\n\n";

        man += "void strobe(float width, float freq);\n";
        man += "void strobe();\n\n";

        man += "void flash(int micros);\n\n";

        man += "void reset();\n\n";

        man += "string time();\n";
        man += "void time(string isoTime);\n\n";

        man += "string debug();\n\n";

        return man;
    }

    else if (command == "debug") {
        return "Debug data not available";
    }

    else {
        return String("Unrecognized command: '") + command + "'";
    }

    return "OK";
}
