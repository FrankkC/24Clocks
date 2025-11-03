
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

void SerialLink::sendLog(const char* data) {
    SerialLink::sendData("MON", data);
}

void SerialLink::sendCommand(const char* data) {
    SerialLink::sendData("CMD", data);
}

void SerialLink::sendData(const char* instruction, const char* data) {
    // We leave space (+2) for the ";" and for "\0"
    char buffer [strlen(instruction) + strlen(data) + 2];
    sprintf(buffer, "%s%s;\0", instruction, data);
    SerialLink::sendData(buffer);
}

void SerialLink::sendData(const char* data) {
    SerialSlave1.println(data);
    SerialSlave2.println(data);
}

bool SerialLink::readCommand(String& commandBuffer) {

    while (SerialSlave2.available()) {
        delay(1);
        if (SerialSlave2.available() > 0) {
            char c = SerialSlave2.read();
            if(c == ';') {
                // Warning: if there was a queued command it would be lost or received truncated. Can it happen?
                return true;
            } else {
                commandBuffer += c;
            }
        }
    }

    return false;

}
