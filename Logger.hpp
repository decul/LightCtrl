#pragma once
#include <MillisTime.hpp>
#include "AnyStream.hpp"

#define LOG_MAX_LENGTH 8192

class Logger {
private:
    static char data[];
    static int16_t writeIndex;
    static int16_t readIndex;
    static bool newErrors;
    static bool overflowed;

    static void AppendLine(const String &str, const char &type) {
        Serial.println(str);
        Write4(DateTime::Now().UnixTime());
        Write(type);
        Put(str + "\n");
    }

    inline static void Write(const uint8_t &byte) {
        data[writeIndex] = byte;
        if (++writeIndex >= LOG_MAX_LENGTH) {
            writeIndex = 0;
            overflowed = true;
        }
    }

    inline static void Write4(const uint32_t &value) {
        for (byte i = 0; i < 4; i++) 
            Write((value >> (i * 8)) & 0xFF);
    }

    static void Put(const String &str) {
        for (int i = 0; i < str.length(); i++) 
            Write(str[i]);
    }

    static void FindReadIndex() {
        if (overflowed) {
            readIndex = writeIndex;
            FindNewLineIndex();
        }
        else {
            readIndex = 0;
        }
    }

    static void FindNewLineIndex() {
        do {
            if (Read() == '\n') {
                // Index is now set on next character after '\n'
                return;
            }
        } while (readIndex != writeIndex);
        // ReadIndex is now equal to writeIndex
    }

    inline static uint8_t Read() {
        uint8_t value = data[readIndex];
        if (++readIndex >= LOG_MAX_LENGTH) 
            readIndex = 0;
        return value;
    }

    static uint32_t Read4() {
        uint32_t value = 0;
        for (byte i = 0; i < 4; i++) 
            value = value | (Read() << (i * 8));
        return value;
    }

    static byte Lvl(const char &type) {
        switch (toupper(type)) {
            case 'D':   return 3;
            case 'I':   return 2;
            case 'E':   return 1;
            default:    return 255;
        } 
    }

    static void PrintTimeStampTo(AnyStream &stream, const DateTime &timeStamp, const DateTime &today) {
        if (timeStamp >= today)
            stream.Print(timeStamp.time().ToString());
        else 
            stream.Print(timeStamp.ToISO());
    }

public:
    static void Info(const String &str) {
        AppendLine(str, 'I');
    }

    static void Debug(const String &str) {
        AppendLine(str, 'D');
    }

    static void Error(const String &str) {
        AppendLine(str, 'E');
        newErrors = true;
    }

    static void PrintTo(AnyStream &stream, const char &type = 'D', const bool &clear = false) {
        byte requestedLvl = Lvl(type);
        DateTime today = DateTime::Now().Day();
        char c;

        stream.Print("Boot time: ");
        PrintTimeStampTo(stream, DateTime::LastResetDate(), today);
        stream.Println();

        FindReadIndex();
        while (readIndex != writeIndex) {
            uint32_t unix = Read4();
            char type = Read();
            if (Lvl(type) > requestedLvl) {
                FindNewLineIndex();
            }
            else {
                DateTime timeStamp(unix);
                PrintTimeStampTo(stream, timeStamp, today);
                stream.Print(String(" ") + type + ": ");
                
                do {
                    c = Read();
                    stream.Print(c);
                } while (c != '\n');
            }
            ESP.wdtFeed();
        }

        if (stream.IsNew())
            stream.Print("Nothing to show");
        stream.Close();

        newErrors = false;
        if (clear) {
            writeIndex = 0;
            overflowed = false;
        }
    }

    static bool AnyNewErrors() {
        return newErrors;
    }

};

int16_t Logger::writeIndex = 0;
int16_t Logger::readIndex = 0;
bool Logger::newErrors = false;
bool Logger::overflowed = false;
char Logger::data[LOG_MAX_LENGTH];