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
            println("Connected to ESP32 Flasher Telnet Log");
        } else {
            _server.available().stop(); // Reject multiple clients
        }
    }
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
