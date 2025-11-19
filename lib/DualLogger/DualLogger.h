#ifndef DUAL_LOGGER_H
#define DUAL_LOGGER_H

#include <Arduino.h>
#include <WiFi.h>

class DualLogger : public Print {
public:
    DualLogger(uint16_t port = 23);
    void begin();
    void handle();
    
    // Check for incoming data from the client
    int available();
    // Read a byte from the client
    int read();
    // Read a string until newline (helper for commands)
    String readStringUntil(char terminator);

    virtual size_t write(uint8_t c) override;
    virtual size_t write(const uint8_t *buffer, size_t size) override;

private:
    WiFiServer _server;
    WiFiClient _client;
};

#endif
