
#include <Arduino.h>
#include "serialLink.h"

SerialLink::SerialLink() {}

void SerialLink::init() {
    MASTER.begin(115200);
}

void SerialLink::sendLog(const String &s) {
    SerialLink::sendData("MON"+s+';');
}

void SerialLink::sendCommand(const String &s) {
    SerialLink::sendData("CMD"+s+';');
}

void SerialLink::sendData(const String &s) {
    MASTER.println(s);
}

bool SerialLink::readCommand(String& commandBuffer) {

    while (MASTER.available()) {
        delay(1);
        if (MASTER.available() > 0) {
            char c = MASTER.read();
            if(c == '\n' || c == '\r') {
                // Just skip
            } else if(c == ';') {
                // Command end
                return true;
            } else {
                commandBuffer += c;
            }
        }
    }

    return false;

}
