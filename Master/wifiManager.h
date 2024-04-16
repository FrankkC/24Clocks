#ifndef wifiManager_h
#define wifiManager_h

#include <Arduino.h>
#include <WiFi.h>


#define WIFI_TIMEOUT 10000 // mS

class WifiManager {

    public:

        WifiManager();
        static void init(void (*callback)());
        static void sendData(String data);
        static String readCommand();

    private:

        static void initWifi();
        static void initServer();

};

#endif
