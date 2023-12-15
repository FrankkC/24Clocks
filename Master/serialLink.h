#ifndef serialLink_h
#define serialLink_h
#include <Arduino.h>

// Invece di avere un solo SLAVE qui devo definire un array o una mappa di slave. Il valore è il serial monitor da usare
// Ho anche bisogno di poter definire porte diverse per l'invio e per la ricezione


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
