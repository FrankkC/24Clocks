
#include <Arduino.h>
#include "serialLink.h"

SerialLink::SerialLink() {}

void SerialLink::init(HardwareSerial* serialLink) {
    serialLink->begin(115200);
}

void SerialLink::sendLog(HardwareSerial* serialLink, const String &s) {
    SerialLink::sendData(serialLink, "MON"+s+';');
}

void SerialLink::sendCommand(HardwareSerial* serialLink, const String &s) {
    SerialLink::sendData(serialLink, "CMD"+s+';');
}

void SerialLink::sendData(HardwareSerial* serialLink, const String &s) {
    serialLink->println(s);
}

bool SerialLink::readCommand(HardwareSerial* serialLink, String& commandBuffer) {

    while (serialLink->available()) {
        delay(1);
        if (serialLink->available() > 0) {
            char c = serialLink->read();
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
