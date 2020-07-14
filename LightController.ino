#include "Light.h"
#include "Button.h"
#include <Arduino.h>
#include <MillisTime.h>
#include "MyEEPROM.h"
#include "SerialMsgr.h"
#include "WiFiMsgr.h"
#include "AnyStream.h"

#define k * 1000
#define LED_HIGH 0
#define LED_LOW  1
#define LED_ESP  D0 // D4 // GPIO2
#define LED_NODE D0 // GPIO16

#define DATE_UPDATE_HOUR 5


Light light;
Button button(D4);
AnyStream serialStream;

bool ledOff = 1;
byte ledCounter = 0;
MicrosTimer ledTimer(0);
MillisTimer dateUpdateTimer(0);
byte dateUpdateFailCount = 0;
DateTime bootTime;


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
    //light.HandleAutoDimming();
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
        AnyStream stream(client);
        String response = HandleCommand(command, stream);
        if (stream.IsNew())
            stream.Print(response);
        stream.Close();
    }
}

void CheckSerialMsgs() { 
    if (Serial.available()) {
        String command = SerialMsgr::ReadMsg();
        String response = HandleCommand(command, serialStream);
        if (serialStream.IsNew())
            serialStream.Println(response);
        serialStream.Restart();
    }
}

void CheckLED() {
    if (ledTimer.HasExpired()) {
        digitalWrite(LED_ESP, ledOff = !ledOff);
        if (!WiFiMsgr::IsConnected()) 
            ledTimer.AddTime(500 k);
        else if (Logger::AnyNewErrors())
            ledTimer.AddTime(ledOff && (++ledCounter & 1) ? 1850 k : 50 k);
        else
            ledTimer.AddTime(ledOff ? 999900 : 100);

        // if (!digitalRead(LED_ESP))
        //     digitalWrite(LED_ESP, LED_LOW);
        // ledTimer.AddTime(1000 k);
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
            bool firstUpdate = !DateTime::IsSet();
            DateTime::Set(date);
            if (firstUpdate) {
                Serial.println("Date Updated");
                bootTime = DateTime::LastResetDate();
            }
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



String HandleCommand(String input, AnyStream &stream) {
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
            default:    stream.Respond("Wrong number of arguments", 400);       break;
        } 
    }

    else if (command == "output") {
        if (argsNo == 1) 
            analogWrite(D1, args[0].toInt());
        else 
            stream.Respond("Wrong number of arguments", 400);
    }

    else if (command == "dimmer") {
        if (argsNo == 0)
            stream.Respond("No arguments given", 400);
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
            stream.Respond("Invalid arguments", 400);
    }

    else if (command == "strobe") {
        if (argsNo == 0)
            light.StopStrobe();
        else if (argsNo == 2)
            light.StartStrobe(args[0].toFloat(), args[1].toFloat());
        else 
            stream.Respond("Wrong number of arguments", 400);
    }

    else if (command == "flash") {
        if (argsNo == 0)
            stream.Respond("No arguments given", 400);
        else 
            light.Flash(args[0].toInt());
    }

    else if (command == "reset") {
        stream.Respond("=== Software Reset ===");
        while (true);
    }

    else if (command == "time") {
        if (argsNo == 0)
            return DateTime::Now().ToISO();
        else if (DateTime::FromISO(args[0]).UnixTime() != 0)
            DateTime::Set(DateTime::FromISO(args[0]));
    }

    else if (command == "?" || command == "help") {
        stream.Println("void on();");
        stream.Println("void off();");
        stream.Println("void switch();\n");

        stream.Println("string color();");
        stream.Println("float color(int led);");
        stream.Println("void color(int led, float value);");
        stream.Println("void color(float col[5]);\n");

        stream.Println("void dimmer([on/off/setdefcol/gettime]);");
        stream.Println("void dimmer([settime], string iso);\n");

        stream.Println("void strobe(float width, float freq);");
        stream.Println("void strobe();");
        stream.Println("void flash(int micros);\n");

        stream.Println("void reset();\n");

        stream.Println("string time();");
        stream.Println("void time(string isoTime);\n");

        stream.Println("string wifi();");
        stream.Println("> void rssi();\n");

        stream.Println("string gui();\n");

        stream.Println("string log([e/i/d/c/ ]);\n");
    }

    else if (command == "wifi") {
        stream.Println("Status: " + WiFiMsgr::Status());
        stream.Println("Signal: " + WiFiMsgr::RSSI());
        stream.Println("IP: " + WiFi.localIP().toString());
        stream.Println("MAC: " + WiFi.macAddress());
    }

    else if (command == "rssi") {
        WiFiMsgr::displayRssi = !WiFiMsgr::displayRssi;
    }

    else if (command == "" || command == "gui") {
        stream.Println("<body style='background: #151515;'>");
        stream.Println("<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>");
        stream.Println("<script type='text/javascript' src='https://decul.github.io/LightCtrl/scripts.js'></script>");
        stream.Println("<script>loadSite()</script></body>");
    }

    else if (command == "log") {
        if (argsNo == 0)
            Logger::PrintTo(stream);
        else if (args[0] == "c")
            Logger::PrintTo(stream, 'D', true);
        else if (args[0].length() == 1)
            Logger::PrintTo(stream, args[0][0]);
        else 
            stream.Respond("Invalid arguments", 400);
    }

    else {
        stream.Respond("Unrecognized command: '" + command + "'", 400);
    }

    return "OK";
}
