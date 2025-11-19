#include "DualLogger.h"

DualLogger::DualLogger(uint16_t port) : _server(port) {}

void DualLogger::begin() {
    _server.begin();
    _server.setNoDelay(true);
}

void DualLogger::handle() {
    if (_server.hasClient()) {
        if (!_client || !_client.connected()) {
            if (_client) _client.stop();
            _client = _server.available();
            println("Connected to ESP32 Telnet Log");
            // Flush input buffer on new connection
            while(_client.available()) _client.read();
        } else {
            _server.available().stop(); // Reject multiple clients
        }
    }
}

int DualLogger::available() {
    if (_client && _client.connected()) {
        return _client.available();
    }
    return 0;
}

int DualLogger::read() {
    if (_client && _client.connected()) {
        return _client.read();
    }
    return -1;
}

String DualLogger::readStringUntil(char terminator) {
    if (_client && _client.connected()) {
        return _client.readStringUntil(terminator);
    }
    return "";
}

size_t DualLogger::write(uint8_t c) {
    Serial.write(c);
    if (_client && _client.connected()) {
        _client.write(c);
    }
    return 1;
}

size_t DualLogger::write(const uint8_t *buffer, size_t size) {
    Serial.write(buffer, size);
    if (_client && _client.connected()) {
        _client.write(buffer, size);
    }
    return size;
}
