#ifndef serialLink_h
#define serialLink_h

#include <Arduino.h>

#define SL_MAX_COMMAND_LENGTH 20

typedef void (*commandCallback)(const char*);

class SerialLink {
public:
    SerialLink(HardwareSerial& serial);
    void setCommandCallback(commandCallback callback);
    void loop();
    void sendCommand(const char* command, const char* data);

private:
    HardwareSerial& _serial;
    commandCallback _callback;
    char _buffer[SL_MAX_COMMAND_LENGTH + 1];
    uint8_t _bufferIndex = 0;

    void processBuffer();
};

#endif