
#include <Arduino.h>
#include "serialLink.h"

SerialLink::SerialLink() {}

void SerialLink::init() {
    MASTER_SLAVE_SERIAL.begin(115200);
    sendLog("Hello Slave!");
}

void SerialLink::sendLog(const String &s) {
    SerialLink::sendData("MON"+s+';');
}

void SerialLink::sendCommand(const String &s) {
    SerialLink::sendData("CMD"+s+';');
}

void SerialLink::sendData(const String &s) {
    MASTER_SLAVE_SERIAL.println(s);
}

bool SerialLink::readCommand(String& commandBuffer) {

    while (MASTER_SLAVE_SERIAL.available()) {
        delay(1);
        if (MASTER_SLAVE_SERIAL.available() > 0) {
            char c = MASTER_SLAVE_SERIAL.read();
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
