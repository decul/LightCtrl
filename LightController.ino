#include "Light.h"
#include "Button.h"
#include <Arduino.h>
#include <MillisTime.h>
#include "MyEEPROM.h"
#include "SerialMsgr.h"
#include "WiFiMsgr.h"


MyEEPROM memory;

Light light;
Button button(A0);


//declare reset function @ address 0
void (*reset) (void) = 0; 


void setup() {
    Serial.begin(115200);
    WiFiMsgr::Initialize();
    light.ResetDimmer();
}

void loop() {
    HandleWebRequests();
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



void HandleWebRequests() {
    WiFiMsgr::CheckConnection();
    MyWiFiClient client = WiFiMsgr::Client();
    if (client) {                             
        String command = WiFiMsgr::ReadMsg(client);
        String response = HandleCommand(command);
        WiFiMsgr::SendResponse(client, response);
    }
}

void serialEvent() { 
    String command = SerialMsgr::ReadMsg();
    String response = HandleCommand(command);
    Serial.println(response);
}



String HandleCommand(String input) {
    String command;
    String args[MAX_CMD_ARGS_LEN];
    byte argsNo = SerialMsgr::SplitCommand(input, command, args);

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

        man += "string gui();\n\n";

        man += "string debug();\n\n";

        return man;
    }

    if (command == "" || command == "gui") {
        return String("<body style='background: #151515;'>") +
            "<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>" +
            "<script type='text/javascript' src='https://decul.github.io/LightCtrl/scripts.js'></script>" +
            "<script>loadSite()</script></body>";
    }

    else if (command == "debug") {
        return "Debug data not available";
    }

    else {
        return String("Unrecognized command: '") + command + "'";
    }

    return "OK";
}
