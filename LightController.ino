#include "Light.h"
#include "Button.h"
#include <Arduino.h>
#include <MillisTime.h>
#include "MyEEPROM.h"
#include "SerialMsgr.h"
#include "WiFiMsgr.h"

#define LED_HIGH 0
#define LED_LOW  1
#define LED_ESP  D4 // GPIO2
#define LED_NODE D0 // GPIO16

#define DATE_UPDATE_HOUR 5


Light light;
Button button(A0);

bool ledOff = 1;
MillisTimer ledTimer(0);
MillisTimer dateUpdateTimer(0);
byte dateUpdateFailCount = 0;


void setup() {
    pinMode(LED_ESP, OUTPUT);

    SerialMsgr::Initialize();
    WiFiMsgr::Initialize();
    MyEEPROM::Initialize();

    light.ResetDimmer();
}

void loop() {
    WiFiMsgr::CheckConnection();
    CheckSerialMsgs();
    CheckWebRequests();
    CheckDateUpdate();
    light.HandleStrobe();  
    light.HandleAutoDimming();
    CheckLED();
    
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



void CheckWebRequests() {
    WiFiClient client = WiFiMsgr::Client();
    if (client) {              
        String command = WiFiMsgr::ReadMsg(client);
        String response = HandleCommand(command);
        WiFiMsgr::SendResponse(client, response);
    }
}

void CheckSerialMsgs() { 
    if (Serial.available()) {
        String command = SerialMsgr::ReadMsg();
        String response = HandleCommand(command);
        Serial.println(response);
    }
}

void CheckLED() {
    if (ledTimer.HasExpired()) {
        digitalWrite(LED_ESP, ledOff = !ledOff);
        if (!WiFiMsgr::IsConnected()) 
            ledTimer.AddTime(500);
        else if (Logger::AnyNewErrors()) 
            ledTimer.AddTime(ledOff ? 1950 : 50);
        else 
            ledTimer.AddTime(ledOff ? 999 : 1);
    }
}

static bool UpdateDate(bool retry = false) {
    HTTPClient http;
    http.begin("http://worldtimeapi.org/api/timezone/Europe/Warsaw.txt");
    int code = http.GET();
    if (code == 200) {
        String response = http.getString();
        response.remove(0, response.indexOf("datetime: ") + 10);
        response.remove(response.indexOf("+"));
        DateTime date = DateTime::FromISO(response);
        if (date.UnixTime() != 0) {
            if (!DateTime::IsSet())
                Serial.println("Date Updated");
            DateTime::Set(date);
            dateUpdateFailCount = 0;
            dateUpdateTimer.Start(Time(DATE_UPDATE_HOUR));
            return true;
        }
        else {
            Logger::Info("Date Update Failed: Parse Error");
        }
    }
    else {
        Logger::Info("Date Update Failed: " + String(code));
    }

    dateUpdateTimer.Start(Time(DATE_UPDATE_HOUR));
    if (retry) {
        if (++dateUpdateFailCount < 5) {
            dateUpdateTimer.Start(5000);
        }
        else {
            Logger::Error("Date Update Failed");
            dateUpdateFailCount = 0;
        }
    }

    return false;
}

static void CheckDateUpdate() {
    if (dateUpdateTimer.HasExpired() && WiFiMsgr::IsConnected()) {
        UpdateDate(true);
    }
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
            return MyEEPROM::GetDefaultDimEndTime().ToString();
        else if (args[0] == "settime" && argsNo == 2)
            MyEEPROM::SetDefaultDimEndTime(Time::FromString(args[1]));
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
        //Serial.println("=== Software Reset ===");
        while (true);
    }

    else if (command == "time") {
        if (argsNo == 0)
            return DateTime::Now().ToISO();
        else if (DateTime::FromISO(args[0]).UnixTime() != 0)
            DateTime::Set(DateTime::FromISO(args[0]));
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

        man += "string wifi();\n";
        man += "> void rssi();\n\n";

        man += "string gui();\n\n";

        man += "string log([e/i/d/c/ ]);\n\n";

        return man;
    }

    else if (command == "wifi") {
        return "Status: " + WiFiMsgr::Status() + 
            "\nSignal: " + WiFiMsgr::RSSI() +
            "\nIP: " + WiFi.localIP().toString() + 
            "\nMAC: " + WiFi.macAddress();
    }

    else if (command == "rssi") {
        WiFiMsgr::displayRssi = !WiFiMsgr::displayRssi;
    }

    else if (command == "" || command == "gui") {
        return String("<body style='background: #151515;'>") +
            "<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>" +
            "<script type='text/javascript' src='https://decul.github.io/LightCtrl/scripts.js'></script>" +
            "<script>loadSite()</script></body>";
    }

    else if (command == "log") {
        if (argsNo == 0)
            return Logger::Read();
        else if (args[0] == "c")
            return Logger::Read('D', true);
        else if (args[0].length() == 0)
            return Logger::Read(args[0][0]);
        else 
            return "Invalid arguments";
    }

    else {
        return String("Unrecognized command: '") + command + "'";
    }

    return "OK";
}
