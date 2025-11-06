#include "AVRFlasher.h"
#include <Arduino.h>

AVRFlasher::AVRFlasher(HardwareSerial& serial, int reset_pin, int rx_pin, int tx_pin) : _serial(serial), _reset_pin(reset_pin), _rx_pin(rx_pin), _tx_pin(tx_pin) {
}

void AVRFlasher::resetTarget() {
    pinMode(_reset_pin, OUTPUT);
    digitalWrite(_reset_pin, HIGH);
    delay(100);
    digitalWrite(_reset_pin, LOW);
    delay(50);
    digitalWrite(_reset_pin, HIGH);
    delay(100); // Wait for bootloader to initialize
}

// Main public method to start flashing
bool AVRFlasher::flash(const uint8_t* firmware, size_t size) {
    _serial.begin(115200, SERIAL_8N1, _rx_pin, _tx_pin);
    resetTarget();

    if (!sync()) {
        Serial.println("Flasher: Failed to sync with bootloader.");
        _serial.end();
        return false;
    }

    Serial.println("Flasher: Synced with bootloader.");

    read_signature(); // Optional: check if the signature matches ATmega2560

    bool success = program_flash(firmware, size);

    leaveProgMode();
    _serial.end();

    if (success) {
        Serial.println("Flasher: Firmware flashed successfully!");
    } else {
        Serial.println("Flasher: Firmware flashing failed.");
    }

    return success;
}

// Establish synchronization with the bootloader
bool AVRFlasher::sync() {
    _serial.flush();
    for (int i = 0; i < 10; i++) {
        _serial.write(STK_GET_SYNC);
        _serial.write(SYNC_CRC_EOP);
        if (getch() == STK_INSYNC) {
            if (getch() == STK_OK) {
                return true;
            }
        }
        delay(50);
    }
    return false;
}

// Read a single character from serial with a timeout
uint8_t AVRFlasher::getch() {
    unsigned long start = millis();
    while (millis() - start < 1000) { // 1 second timeout
        if (_serial.available()) {
            return _serial.read();
        }
    }
    return STK_NOSYNC; // Return a value indicating timeout/failure
}

void AVRFlasher::leaveProgMode() {
    _serial.write(STK_LEAVE_PROGMODE);
    _serial.write(SYNC_CRC_EOP);
    getch(); // STK_INSYNC
    getch(); // STK_OK
}

void AVRFlasher::read_signature() {
    _serial.write(STK_READ_SIGN);
    _serial.write(SYNC_CRC_EOP);
    if (getch() == STK_INSYNC) {
        uint8_t sig2 = getch();
        uint8_t sig1 = getch();
        uint8_t sig0 = getch();
        if (getch() == STK_OK) {
            Serial.print("Flasher: Signature is ");
            Serial.print(sig0, HEX);
            Serial.print(" ");
            Serial.print(sig1, HEX);
            Serial.print(" ");
            Serial.println(sig2, HEX);
        }
    }
}

// High-level function to handle the entire programming process
bool AVRFlasher::program_flash(const uint8_t* firmware, size_t size) {
    return parse_hex_and_program(firmware, size);
}

// Helper to convert two hex chars to a byte
static uint8_t hex_char_to_byte(char c1, char c2) {
    uint8_t val = 0;
    if (c1 >= '0' && c1 <= '9') val += (c1 - '0') * 16;
    else if (c1 >= 'A' && c1 <= 'F') val += (c1 - 'A' + 10) * 16;
    else if (c1 >= 'a' && c1 <= 'f') val += (c1 - 'a' + 10) * 16;

    if (c2 >= '0' && c2 <= '9') val += (c2 - '0');
    else if (c2 >= 'A' && c2 <= 'F') val += (c2 - 'A' + 10);
    else if (c2 >= 'a' && c2 <= 'f') val += (c2 - 'a' + 10);
    return val;
}

// Parse the Intel HEX file format and program the flash memory
bool AVRFlasher::parse_hex_and_program(const uint8_t* hex_data, size_t size) {
    uint32_t current_pos = 0;
    uint32_t line_start = 0;
    uint32_t extended_addr = 0;

    while (current_pos < size) {
        // Find the start of a line
        if (hex_data[current_pos] == ':') {
            line_start = current_pos;

            uint8_t byte_count = hex_char_to_byte(hex_data[line_start + 1], hex_data[line_start + 2]);
            uint16_t address = (hex_char_to_byte(hex_data[line_start + 3], hex_data[line_start + 4]) << 8) |
                               hex_char_to_byte(hex_data[line_start + 5], hex_data[line_start + 6]);
            uint8_t record_type = hex_char_to_byte(hex_data[line_start + 7], hex_data[line_start + 8]);

            if (record_type == 0x00) { // Data Record
                uint32_t full_addr = extended_addr + address;

                // Set load address
                _serial.write(STK_LOAD_ADDRESS);
                _serial.write((full_addr >> 1) & 0xFF);
                _serial.write((full_addr >> 9) & 0xFF);
                _serial.write(SYNC_CRC_EOP);
                if (getch() != STK_INSYNC || getch() != STK_OK) return false;

                // Program page data
                _serial.write(STK_PROG_PAGE);
                _serial.write(0x00); // high byte of size
                _serial.write(byte_count); // low byte of size
                _serial.write('F'); // Flash memory type

                for (int i = 0; i < byte_count; i++) {
                    uint8_t data_byte = hex_char_to_byte(hex_data[line_start + 9 + i * 2], hex_data[line_start + 10 + i * 2]);
                    _serial.write(data_byte);
                }
                _serial.write(SYNC_CRC_EOP);
                if (getch() != STK_INSYNC || getch() != STK_OK) return false;

            } else if (record_type == 0x02) { // Extended Segment Address Record
                extended_addr = (hex_char_to_byte(hex_data[line_start + 9], hex_data[line_start + 10]) << 16) |
                                (hex_char_to_byte(hex_data[line_start + 11], hex_data[line_start + 12]) << 8);
                extended_addr <<= 4;
            } else if (record_type == 0x04) { // Extended Linear Address Record
                 extended_addr = (hex_char_to_byte(hex_data[line_start + 9], hex_data[line_start + 10]) << 24) |
                                (hex_char_to_byte(hex_data[line_start + 11], hex_data[line_start + 12]) << 16);
            } else if (record_type == 0x01) { // End of File Record
                return true; // Success
            }
            // Other record types (03, 05) are ignored
        }
        
        // Move to the next line
        while(current_pos < size && hex_data[current_pos] != '\n') {
            current_pos++;
        }
        current_pos++;
    }

    return true; // Should be reached if the hex file is well-formed
}
