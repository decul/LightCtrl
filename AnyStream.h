#pragma once
#include <ESP8266WiFi.h>

#define ANY_STREAM_NEW 0
#define ANY_STREAM_OPENED 1
#define ANY_STREAM_CLOSED 2

class AnyStream {
private:
    WiFiClient *client = nullptr;
    byte state = ANY_STREAM_NEW;

public:
    AnyStream() {

    }

    AnyStream(WiFiClient &_client) {
        client = &_client;
    }

    void Respond(const String &str, const int16_t &code = 200) {
        Start(code);
        Println(str);
        Close();
    }

    void Start(const int16_t &code) {
        if (client != nullptr && state == ANY_STREAM_NEW) {
            client->print("HTTP/1.1 ");
            client->println(code);
            client->println("Access-Control-Allow-Origin: *");
            client->println("Content-type:text/html");
            client->println("Connection: close");
            client->println();
            state = ANY_STREAM_OPENED;
        }
    }

    void Print(const String &str) {
        if (client != nullptr) {
            Start(200);
            client->print(str);
        }
        else {
            Serial.print(str);
            state = ANY_STREAM_OPENED;
        }
    }

    void Print(const char &c) {
        Print(String(c));
    }

    void Println(const String &str) {
        Print(str);
        Print('\n');
    }

    void Println() {
        Print('\n');
    }

    void Close() {
        if (client != nullptr) {
            client->println("\n");
            client->stop();
            state = ANY_STREAM_CLOSED;
        }
    }

    void Restart() {
        if (client == nullptr) {
            state = ANY_STREAM_NEW;
        }
    }

    bool IsNew() {
        return state == ANY_STREAM_NEW;
    }
};