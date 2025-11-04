#ifndef AVRFLASHER_H
#define AVRFLASHER_H

#include <Arduino.h>

class AVRFlasher {
public:
    // Constructor takes the serial port for slaves and the reset pin for the specific slave
    AVRFlasher(HardwareSerial& serial, int reset_pin);

    // Flashes the firmware. The firmware is a byte array containing the Intel HEX data.
    bool flash(const uint8_t* firmware, size_t size);

private:
    HardwareSerial& _serial;
    int _reset_pin;

    void resetTarget();
    bool sync();
    void leaveProgMode();
    uint8_t getch();
    void read_signature();

    bool program_flash(const uint8_t* firmware, size_t size);
    bool parse_hex_and_program(const uint8_t* hex_data, size_t size);

    // STK500 Commands and constants
    static const uint8_t STK_OK = 0x10;
    static const uint8_t STK_INSYNC = 0x14;
    static const uint8_t STK_GET_PARAMETER = 0xA1;
    static const uint8_t STK_SET_DEVICE = 0xB6;
    static const uint8_t STK_ENTER_PROGMODE = 0x50;
    static const uint8_t STK_LEAVE_PROGMODE = 0x51;
    static const uint8_t STK_LOAD_ADDRESS = 0x55;
    static const uint8_t STK_PROG_PAGE = 0x64;
    static const uint8_t STK_READ_SIGN = 0x75;
    static const uint8_t STK_NOSYNC = 0x00;
    static const uint8_t SYNC_CRC_EOP = 0x20;

    // AVR Device properties (for ATmega2560)
    static const uint16_t AVR_PAGE_SIZE = 256; // Page size in bytes
    static const uint8_t AVR_DEVICE_CODE = 0x44;
    static const uint8_t AVR_SIGNATURE_0 = 0x1E;
    static const uint8_t AVR_SIGNATURE_1 = 0x98;
    static const uint8_t AVR_SIGNATURE_2 = 0x01;
};

#endif // AVRFLASHER_H
