#ifndef serialLink_h
#define serialLink_h
#include <Arduino.h>

class SerialLink {

    public:

        SerialLink();
        static void init(HardwareSerial* serialLink);
        static void sendLog(HardwareSerial* serialLink, const String &s);
        static void sendCommand(HardwareSerial* serialLink, const String &s);
        static bool readCommand(HardwareSerial* serialLink, String& commandBuffer);

    private:
        static void sendData(HardwareSerial* serialLink, const String &s);

};

#endif
