#ifndef serialLink_h
#define serialLink_h
#include <Arduino.h>

// Instead of having a single SLAVE here I have to define an array or a map of slaves. The value is the serial monitor to use
// I also need to be able to define different ports for sending and receiving


class SerialLink {

    public:
        SerialLink();
        static void init();
        static void sendLog(const char* data);
        static void sendCommand(const char* data);
        static bool readCommand(String& commandBuffer);

    private:
        static void sendData(const char* instruction, const char* data);
        static void sendData(const char* data);

};

#endif
