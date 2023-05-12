#ifndef wifiManager_h
#define wifiManager_h
#include <Arduino.h>

#define TIMEOUT 10000 // mS
#define WIFI_SERIAL Serial

class WifiManager {

    public:

        WifiManager();
        static void init();
        static void sendData(String str);
        static String readCommand();

    private:

        static bool sendCommand(String cmd, String ack = "", String error = "");
        static bool echoFind(String keyword, String error);
        static bool initWifi();
        static bool initServer();

};

#endif
