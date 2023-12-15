#ifndef serialLink_h
#define serialLink_h
#include <Arduino.h>

#define MASTER Serial3

class SerialLink {

    public:

        SerialLink();
        static void init();
        static void sendLog(const String &s);
        static void sendCommand(const String &s);
        static bool readCommand(String& commandBuffer);

    private:
        static void sendData(const String &s);

};

#endif
