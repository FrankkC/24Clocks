/*
 * AVR Flash Library for ESP32
 * Implements STK500 protocol to flash ATmega2560
 * 
 * Based on code from Laukik Hase's project:
 * https://github.com/ESP32-Musings/OTA_update_AVR_using_ESP32
 */

#ifndef AVR_FLASH_ARDUINO_H
#define AVR_FLASH_ARDUINO_H

#include <Arduino.h>

#define MAX_FIRMWARE_SIZE 262144  // ATmega2560 flash size

// Global debugger stream pointer (defaults to &Serial)
extern Print* avr_debugger;

/**
 * Configuration structure for AVR flashing pins
 */
struct AVRFlashConfig {
    uint8_t reset_pin;
    uint8_t tx_pin;
    uint8_t rx_pin;
    uart_port_t uart_num;
};

/**
 * Flash firmware to AVR using STK500 protocol
 * @param firmware_hex Intel HEX format firmware as null-terminated string
 * @param config Pin configuration for the target AVR
 * @return true if successful, false otherwise
 */
bool flash_avr_firmware(const char* firmware_hex, const AVRFlashConfig& config);

#endif
