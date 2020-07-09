#include <MillisTime.h>

#define LOG_MAX_LENGTH 10000

class Logger {
private:
    static String data;
    static bool newErrors;

    static void Append(String str, char type) {
        Serial.println(str);
        String line = String("    ") + type + str + "\n";
        uint32_t unix = DateTime::Now().UnixTime();
        for (int i = 0; i < 4; i++)
            line[i] = (unix >> (i * 8)) & 0xFF;
        //data += line;
        while (data.length() > 10000)
            data.remove(0, data.indexOf('\n') + 1);
    }

    static byte Lvl(char type) {
        switch (toupper(type)) {
            case 'D':   return 3;
            case 'I':   return 2;
            case 'E':   return 1;
            default:    return 255;
        } 
    }

public:
    static void Info(String str) {
        Append(str, 'I');
    }

    static void Debug(String str) {
        Append(str, 'D');
    }

    static void Error(String str) {
        Append(str, 'E');
        newErrors = true;
    }

    static String Read(char type = 'D', bool clear = false) {
        byte requestedLvl = Lvl(type);
        DateTime today = DateTime::Now().Day();
        String result = "";

        int index = 0;
        while (index < data.length()) {
            byte lvl = Lvl(data[index + 4]);
            if (lvl > requestedLvl) {
                index = data.indexOf("\n", index) + 1;
            }
            else {
                uint32_t unix = 0;
                for (int i = 0; i < 4; i++) 
                    unix = unix | ((uint32_t)data[index++] << (i * 8));
                DateTime timeStamp(unix);
                if (timeStamp >= today)
                    result += timeStamp.time().ToString();
                else 
                    result += timeStamp.ToISO();

                result += String(" ") + data[index++] + ": ";

                int splIndex = data.indexOf("\n", index);
                if (splIndex < 0)
                    splIndex = data.length();

                result += data.substring(index, splIndex) + "\n";
                index = splIndex + 1;
            }
        }

        newErrors = false;
        if (clear)
            data.remove(0);

        if (result.length() == 0)
            result += "Nothing to show";

        return result;
    }

    static bool AnyNewErrors() {
        return newErrors;
    }

};

String Logger::data = "";
bool Logger::newErrors = false;