/*
 * AVR Flash Library for ESP32
 * Implements STK500 protocol to flash ATmega2560
 */

#ifndef AVR_FLASH_ARDUINO_H
#define AVR_FLASH_ARDUINO_H

#include <Arduino.h>

#define MAX_FIRMWARE_SIZE 262144  // ATmega2560 flash size

/**
 * Flash firmware to AVR using STK500 protocol
 * @param firmware_hex Intel HEX format firmware as null-terminated string
 * @return true if successful, false otherwise
 */
bool flash_avr_firmware(const char* firmware_hex);

#endif
