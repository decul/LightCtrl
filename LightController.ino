#include <IRremote.h>
#include "Xmas.h"
#include "Light.h"
#include <Arduino.h>
#include <RTClib.h>

RTC_DS1307 rtc;

IRrecv irrecv(7);
long prevIRCode = 0;


Light light;
// Xmas xmas;



void setup() {
    Serial.begin(9600);
    irrecv.enableIRIn();
    pinMode(LED_BUILTIN, OUTPUT);
    
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    
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
        Serial.println(irResult.value, HEX);
        irrecv.resume(); // Receive the next value
    }
}



void handleIrCode(long code) {
    switch (code) {
        case 0xFFA857:  light.Switch(0);     break;
        case 0xFF28D7:  light.Switch(1);     break;  
        case 0xFF6897:  light.Switch(2);     break;  
        case 0xFFE817:  light.Switch(3);     break;  
        
        // case 0xFFA05F:  xmas.Enable();          break;  // UP
        // case 0xFF20DF:  xmas.Disable();
        //                 light.UpdateOutput();   break;  // DOWN


        default:

            if (code == 0xFFFFFFFF)
                code = prevIRCode;
            prevIRCode = code;
    
            switch (code) {
                case 0xFFB04F:  light.Darken(0);   break;
                case 0xFF906F:  light.Brighten(0);   break;
                case 0xFF30CF:  light.Darken(1);   break;
                case 0xFF10EF:  light.Brighten(1);   break;
                case 0xFF708F:  light.Darken(2);   break;
                case 0xFF50AF:  light.Brighten(2);   break;
                case 0xFFF00F:  light.Darken(3);   break;
                case 0xFFD02F:  light.Brighten(3);   break;

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
    String command = "";

    while (Serial.available()) {
        char c = Serial.read();
        if (c == ';') 
            break;
        command += c;

        if (!Serial.available())
            delay(20);
    }

    command.trim();

    if (command.length() > 0) {
        command.toLowerCase();
        handleCommand(command);
        command.remove(0);
    }
}

void handleCommand(String input) {
    String args[4];
    int argIndex = 0;
    
    while (input.length() > 0) {
        int spaceIndex = input.indexOf(' ');
        if (spaceIndex > -1) {
            args[argIndex] = input.substring(0, spaceIndex);
            input.remove(0, spaceIndex + 1);
            argIndex++;
        }
        else {
            args[argIndex] = input;
            break;
        }
    }

    if (args[0] == "time") {
        if (args[1].length() == 0)
            Serial.print((timeToISO(rtc.now()) + "\n").c_str());
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
        if (args[2].length() > 0)
            light.SetColor(args[1].toInt(), args[2].toFloat());
        else if (args[1].length() > 0)
            Serial.println(light.GetColor(args[1].toInt()));
        else 
            Serial.println(light.GetColor());
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
            Serial.println(light.alpha);
    }

    else {
        Serial.println(String("Unrecognized command: '") + args[0] + "'");
    }

    // else if (command.startsWith("+")) {
    //     light.Brighten(command.substring(1).toInt());
    // }
    // else if (command.startsWith("-")) {
    //     light.Darken(command.substring(1).toInt());
    // }
    // else if (command.startsWith("*")) {
    //     light.Switch(command.substring(1).toInt());
    // }
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