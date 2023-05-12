
#include <Arduino.h>
#include "serialLink.h"

SerialLink::SerialLink() {}

void SerialLink::init() {
    MASTER_SLAVE_SERIAL.begin(115200);
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