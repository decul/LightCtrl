#include "XmasLight.hpp"
#include "Button.hpp"
#include <MillisTime.hpp>
#include "MyEEPROM.hpp"
#include "SerialMsgr.hpp"
#include "WiFiMsgr.hpp"
#include "AnyStream.hpp"
#include <ESP8266HTTPClient.h>

#define k * 1000
#define LED_HIGH 0
#define LED_LOW  1
#define LED_ESP  D4 // GPIO2
#define LED_NODE D0 // GPIO16

#define DATE_UPDATE_HOUR 5


XmasLight light;
Button button(D3);
AnyStream serialStream;

bool ledOff = 1;
byte ledCounter = 0;
MicrosTimer ledTimer(0);
MillisTimer dateUpdateTimer(0);
byte dateUpdateFailCount = 0;
DateTime bootTime;

void setup() {
    pinMode(LED_NODE, OUTPUT);
    digitalWrite(LED_NODE, LED_LOW);

    SerialMsgr::Initialize();
    WiFiMsgr::Initialize();
    MyEEPROM::Initialize();
}

void loop() {
    WiFiMsgr::CheckConnection();
    CheckSerialMsgs();
    CheckWebRequests();
    CheckDateUpdate();
    light.DimmerHandle();
    light.XmasHandle();
    //CheckLED();
    
    switch (button.GetAction()) {
        case Button::CLICK:
            light.Switch();
            break;

        case Button::HOLD:
            light.AdjustDimmer(0.004);
            break;

        case Button::CLICK_HOLD:
            light.AdjustDimmer(-0.004);
            break;

        case Button::DOWN:
            light.ResetDimmerZone();
            break;

        case Button::TRIPLE_CLICK:
            WiFiMsgr::Reconnect();
            break;

        case Button::QUADRUPLE_CLICK:
            HandleCommand("reset", serialStream);
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
        stream.Close(light.GetColors(), Logger::AnyNewErrors());
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
        digitalWrite(LED_NODE, ledOff = !ledOff);
        if (!WiFiMsgr::IsConnected()) 
            ledTimer.AddTime(500 k);
        else if (Logger::AnyNewErrors())
            ledTimer.AddTime(ledOff && (++ledCounter & 1) ? 1850 k : 50 k);
        else
            ledTimer.AddTime(ledOff ? 999900 : 100);

        // if (!digitalRead(LED_NODE))
        //     digitalWrite(LED_NODE, LED_LOW);
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
            Logger::Debug("Date Updated");
            if (firstUpdate) {
                Serial.println("Date Updated");
                bootTime = DateTime::LastResetDate();
            }
            dateUpdateFailCount = 0;
            dateUpdateTimer.Start(Time(DATE_UPDATE_HOUR), 600);
            return true;
        }
        else {
            Logger::Info("Date Update Failed: Parse Error");
        }
    }
    else {
        Logger::Info("Date Update Failed: " + String(code));
    }

    dateUpdateTimer.Start(Time(DATE_UPDATE_HOUR), 600);
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
            case 2:     light.SetColor(args[0].toInt(), args[1].toFloat());     break;
            case 5:     light.SetColors(args);                                  break;
            case 6:     light.SetColors(args, args[5]);                         break;
            default:    stream.Respond("Wrong number of arguments", 400);       break;
        } 
    }

    else if (command == "output") {
        switch (argsNo) {
            case 0:     return light.GetOutputs();
            case 1:     return String(light.GetOutput(args[0].toInt()));
            case 2:     return String(light.SetOutput(args[0].toInt(), args[1].toInt()));
            default:    stream.Respond("Too many arguments", 400);              break;
        } 
    }

    else if (command == "dimmer") {
        if (argsNo == 0)
            stream.Respond("No arguments given", 400);
        else if (args[0] == "on")
            light.DimmerEnable();
        else if (args[0] == "off")
            light.DimmerDisable();
        else if (args[0] == "skip") {
            int count = (argsNo == 2) ? args[1].toInt() : 1;
            light.DimmerSkip(count);
        }
        else 
            stream.Respond("Invalid arguments", 400);
    }

    else if (command == "day") 
        light.SetDaylight();
    else if (command == "evening")
        light.SetEveningLight();
    else if (command == "dusk" || command == "dawn")    // Leaving 'dawn' for compability
        light.SetDuskLight();

    else if (command == "strobe") {
        if (argsNo == 2) 
            light.StrobeHardware(args[0].toFloat(), args[1].toInt());
        else if (args[0] == "?") 
            StrobeLight::StrobePrintHelpTo(stream);
        else 
            stream.Respond("Wrong arguments", 400);
    }

    else if (command == "rainbow") {
        if (argsNo == 2) 
            light.StrobeHardwareRGB(args[0].toFloat(), args[1].toInt());
        else if (args[0] == "?") 
            StrobeLight::StrobePrintHelpTo(stream);
        else 
            stream.Respond("Wrong arguments", 400);
    }

    else if (command == "flash") {
        if (argsNo == 0)
            stream.Respond("No arguments given", 400);
        else 
            light.Flash(args[0].toInt());
    }

    else if (command == "xmas") {
        if (argsNo == 1)
            light.XmasStart(args[0].toInt());
        else 
            light.XmasStop();
    }

    else if (command == "reset") {
        stream.Respond("=== Software Reset ===");
        stream.Close("", false);
        while (true);
    }

    else if (command == "time") {
        if (argsNo == 0)
            return DateTime::Now().ToISO();
        else if (args[0] == "update")
            UpdateDate();
        else if (DateTime::FromISO(args[0]).UnixTime() != 0)
            DateTime::Set(DateTime::FromISO(args[0]));
        else 
            stream.Respond("Wrong arguments", 400);
    }

    else if (command == "?" || command == "help") {
        stream.Println("void [on/off/switch]\n");

        stream.Println("string color();");
        stream.Println("void color(int led, float value);");
        stream.Println("void color(float col[5/6]);\n");

        stream.Println("void output(int led?, int value?);\n");

        stream.Println("void [day/evening/dusk];\n");

        stream.Println("void dimmer([on/off]);");
        stream.Println("void dimmer([skip], int count?);\n");

        stream.Println("void strobe(float width, int freq);");
        stream.Println("void strobe([?]);");
        stream.Println("void rainbow(float width, int freq);");
        stream.Println("void rainbow([?]);");
        stream.Println("void flash(int micros);\n");

        stream.Println("void reset();\n");

        stream.Println("void time(string isoTime?);\n");

        stream.Println("string wifi();");
        stream.Println("void reconnect();");
        stream.Println("void rssi();\n");

        stream.Println("string gui();");
        stream.Println("string web(string[] command);\n");

        stream.Println("string log([e/i/d/clr/test/ ]);\n");
    }

    else if (command == "wifi") {
        stream.Println("Status: " + WiFiMsgr::Status());
        stream.Println("Signal: " + WiFiMsgr::RSSI());
        stream.Println("IP: " + WiFi.localIP().toString());
        stream.Println("MAC: " + WiFi.macAddress());
    }

    else if (command == "reconnect") {
        WiFiMsgr::Reconnect();
    }

    else if (command == "rssi") {
        WiFiMsgr::displayRssi = !WiFiMsgr::displayRssi;
    }

    else if (command == "" || command == "gui") {
        stream.Start(200, true);
        stream.Println("<body style='background: #151515;'>");
        stream.Println("<script type='text/javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js'></script>");
        stream.Println("<script type='text/javascript' src='https://decul.github.io/LightCtrlGUI/scripts.js'></script>");
        stream.Println("<script>loadSite()</script></body>");
    }

    else if (command == "log") {
        if (argsNo == 0)
            Logger::PrintTo(stream);
        else if (args[0].length() == 1)
            Logger::PrintTo(stream, args[0][0]);
        else if (args[0] == "clr")
            Logger::PrintTo(stream, 'D', true);
        else if (args[0] == "test")
            Logger::Error("Test Error");
        else 
            stream.Respond("Invalid arguments", 400);
    }

    else if (command == "favicon.ico") {
        stream.Start(301, true, "Location: https://decul.github.io/LightCtrlGUI/img/rgb.png");
    }

    else if (command != "status") {
        stream.Respond("Unrecognized command: '" + command + "'", 400);
    }

    return "OK";
}
