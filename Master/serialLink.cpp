
#include <Arduino.h>
#include "serialLink.h"
#include <HardwareSerial.h>

#define RXD1 18
#define TXD1 19

#define RXD2 16
#define TXD2 17

HardwareSerial SerialSlave1(1);
HardwareSerial SerialSlave2(2);

SerialLink::SerialLink() {}

void SerialLink::init() {
    SerialSlave1.begin(115200, SERIAL_8N1, RXD1, TXD1);
    SerialSlave2.begin(115200, SERIAL_8N1, RXD2, TXD2);
}

void SerialLink::sendLog(const String &s) {
    SerialLink::sendData("MON"+s+';');
}

void SerialLink::sendCommand(const String &s) {
    SerialLink::sendData("CMD"+s+';');
}

void SerialLink::sendData(const String &s) {
    SerialSlave1.println(s);
    SerialSlave2.println(s);
}

bool SerialLink::readCommand(String& commandBuffer) {

    while (SerialSlave2.available()) {
        delay(1);
        if (SerialSlave2.available() > 0) {
            char c = SerialSlave2.read();
            if(c == ';') {
                // Warning: se ci fosse un comando accodato verrebbe perso o ricevuto troncato. Può succedere?
                return true;
            } else {
                commandBuffer += c;
            }
        }
    }

    return false;

}
