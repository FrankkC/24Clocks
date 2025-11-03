#include "serialLink.h"

SerialLink::SerialLink(HardwareSerial& serial) : _serial(serial) {}

void SerialLink::setCommandCallback(commandCallback callback) {
    _callback = callback;
}

void SerialLink::loop() {
    while (_callback != nullptr && _serial.available()) {
        char c = _serial.read();
        if (c == ';') {
            _buffer[_bufferIndex] = '\0';
            processBuffer();
            _bufferIndex = 0;
        } else if (_bufferIndex < SL_MAX_COMMAND_LENGTH) {
            _buffer[_bufferIndex++] = c;
        }
    }
}

void SerialLink::sendCommand(const char* command, const char* data) {
    _serial.print(command);
    _serial.print(data);
    _serial.print(';');
}

void SerialLink::processBuffer() {
    if (_callback != nullptr) {
        _callback(_buffer);
    }
}