#pragma once
#ifndef C_CPP
    #include <ESP8266WiFi.h>
#endif

#define ANY_STREAM_NEW 0
#define ANY_STREAM_OPENED 1
#define ANY_STREAM_OPENED_AS_JSON 2
#define ANY_STREAM_OPENED_AS_HTML 3
#define ANY_STREAM_CLOSED 4

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

    void Respond(const String &response, const int16_t &code = 200) {
        Start(code);
        Println(response);
    }

    void Start(const int16_t &code, const bool &asHTML = false, const String &additionalHeaders = "") {
        if (client != nullptr && state == ANY_STREAM_NEW) {
            client->print("HTTP/1.1 ");
            client->println(code);
            client->println("Access-Control-Allow-Origin: *");
            client->println("Connection: close");
            if (additionalHeaders.length() > 0)
                client->println(additionalHeaders);
            if (asHTML)
                client->print("Content-type:text/html\n\n");
            else
                client->print("Content-type:application/json\n\n{\"resp\":\"");
            state = asHTML ? ANY_STREAM_OPENED_AS_HTML : ANY_STREAM_OPENED_AS_JSON;
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

    void Close(const String &colors, bool errors) {
        if (client != nullptr) {
            if (state == ANY_STREAM_OPENED_AS_JSON) {
                client->print("\"");
                if (colors.length() > 0)
                    client->print(",\"col\":\"" + colors + "\"");
                client->print(",\"err\":" + String(errors));
                client->println("}");
            }
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