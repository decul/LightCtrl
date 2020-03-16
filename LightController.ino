#include "Xmas.h"
#include "Light.h"
#include "Button.h"
#include <Arduino.h>
#include "RTClib.h"
#include "MyEEPROM.h"
#include "SerialMsgr.h"
#include "IrMsgr.h"
#include "Memory.h"

RTC_DS1307 rtc;
MyEEPROM memory;


Light light;
Xmas xmas(&light);
Button button(A10);


//declare reset function @ address 0
void (*reset) (void) = 0; 

DateTime startupTime;



void setup() {
    SerialMsgr::Initialize();
    IrMsgr::Initialize();
    
    SerialMsgr::Debug("Arduino Started");
    
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
    }
    if (!rtc.isrunning()) {
        Serial.println("RTC is NOT running!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    startupTime = rtc.now();
    
    light.ResetDimmer();
}



void loop() {
    String irCommand = IrMsgr::GetCommand();
    if (irCommand != "") 
        handleCommand(irCommand);

    light.HandleStrobe();
    light.HandleAutoDimming();

    switch (button.GetAction()) {
        case Button::CLICK:
            light.SwitchPower();
            break;

        case Button::HOLD:
            light.Adjust(4, 0.004);
            break;

        case Button::CLICK_HOLD:
            light.Adjust(4, -0.004);
            break;

        case Button::DOUBLE_CLICK:
            xmas.Switch();
            break;
    }

    xmas.Run();
}



String handleCommand(String input) {
    const byte MAX_ARGS_LEN = 8;
    String args[MAX_ARGS_LEN];
    byte argsNo = 0;

    String command = GetWord(input);

    while (input.length() > 0) {
        if (++argsNo >= MAX_ARGS_LEN) {
            return "Too many arguments given";
        }
        args[argsNo - 1] = GetWord(input);
    }


    if (command == "on") {
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
                light.Adjust(args[0].toInt(), 0.04);
            else if (args[1] == "-")
                light.Adjust(args[0].toInt(), -0.04);
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

    else if (command == "wifi") {
        if (argsNo == 0)
            return "No arguments given";
        String msg = args[0];
        for (int i = 0; i < argsNo; i++)
            msg += " " + args[i];
        SerialMsgr::SendWifiMsg(msg);
        return SerialMsgr::ReadWifiMsg(true);
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

    else if (command == "filter") {
        light.filterEnabled = !light.filterEnabled;
        light.UpdateOutput();
        return light.filterEnabled ? "ON" : "OFF";
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

    else if (command == "xmas") {
        if (argsNo == 0)
            xmas.Switch();
    }

    else if (command == "reset") {
        for (int i = 0; i < 2; i++)
            SerialMsgr::SendMsg(i, "=== Software Reset ===");
        delay(50);
        reset();
    }

    else if (command == "time") {
        if (argsNo == 0)
            return rtc.now().toISO();
        else if (DateTime::FromISO(args[0]).unixtime() != 0)
            rtc.adjust(DateTime::FromISO(args[0]));
    }

    else if (command == "?" || command == "help") {
        String man = "";

        man += "void on();\n";
        man += "void off();\n\n";

        man += "string color();\n";
        man += "float color(int led);\n";
        man += "string color(int led, float value);\n";
        man += "string color(int led, [+/-/switch]);\n";
        man += "string color(float col[5]);\n\n";

        man += "void wifi([help/?]);\n\n";

        man += "void dimmer([on/off/setdefcol/gettime]);\n";
        man += "void dimmer([settime], string iso);\n\n";

        man += "string filter();\n\n";

        man += "void strobe(float width, float freq);\n";
        man += "void strobe();\n\n";

        man += "void flash(int micros);\n\n";

        man += "void xmas();\n\n";

        man += "void reset();\n\n";

        man += "string time();\n";
        man += "void time(string isoTime);\n\n";

        man += "string debug();\n\n";

        return man;
    }

    else if (command == "debug") {
        return "Startup time: " + startupTime.toISO() + "\nAvailable memory: " + String(FreeMemory()) + "B";
    }

    else if (command == "ok" || command == "" || command == "unknown"){
        return "";
    }

    else {
        return String("Unrecognized command: '") + command + "'";
    }

    return "OK";
}

String GetWord(String &input) {
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
