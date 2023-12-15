#ifndef wifiManager_h
#define wifiManager_h

#include <Arduino.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  #include <WiFi.h>
#endif

#define TIMEOUT 10000 // mS

class WifiManager {

    public:

        WifiManager();
        static void init();
        static void sendData(String data);
        static String readCommand();

    private:

        static void initWifi();
        static void initServer();

};

#endif
