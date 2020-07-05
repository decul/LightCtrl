#pragma once
#include <ESP8266WiFi.h>

// Workaround so WiFiClient wouldn't cause Intellisense errors

#ifndef __INTELLISENSE__ 
    #define MyWiFiClient WiFiClient
#else
class MyWiFiClient : public WiFiClient {
public:
    MyWiFiClient(WiFiClient& client);
    ~MyWiFiClient();
    int connect(IPAddress& ip, uint16_t port);
    int connect(const char *host, uint16_t port);
    int connect(const String& host, uint16_t port);
    size_t write(uint8_t);
    size_t write(const uint8_t *buf, size_t size);
    size_t write_P(PGM_P buf, size_t size);
    int available();
    int read();
    int read(uint8_t *buf, size_t size);
    int peek();
    size_t peekBytes(uint8_t *buffer, size_t length);
    bool flush(unsigned int maxWaitMs = 0);
    bool stop(unsigned int maxWaitMs = 0);
    uint8_t connected();
    operator bool();
    connect(IPAddress ip, uint16_t port);
    flush();
};
#endif