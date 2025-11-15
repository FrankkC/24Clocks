#ifndef AVR_FLASH_ARDUINO_H
#define AVR_FLASH_ARDUINO_H

#include <Arduino.h>

// Dimensione massima del firmware (262144 byte per ATmega2560)
#define MAX_FIRMWARE_SIZE 262144 

/**
 * @brief Esegue il flashing del firmware su un dispositivo AVR.
 * 
 * @param firmware_hex L'array di caratteri contenente il firmware in formato Intel HEX.
 * @return true se il flashing e la verifica hanno successo, false altrimenti.
 */
bool flash_avr_firmware(const char* firmware_hex);

#endif // AVR_FLASH_ARDUINO_H
