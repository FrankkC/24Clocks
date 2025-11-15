/*
 * ESP32 AVR Flasher
 * Flashes firmware to ATmega2560 via STK500 protocol
 */

#include "avr_flash_arduino.h"
#include "firmware_slave.h"

void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32 AVR Flasher Ready");
  Serial.println("Press any key to start flashing...");
}

void loop() {
  if (Serial.available()) {
    Serial.read();
    
    Serial.println("\n=== Starting Flash Process ===");
    
    if (flash_avr_firmware(firmware_slave_hex)) {
      Serial.println("\n=== Flash Successful ===");
    } else {
      Serial.println("\n=== Flash Failed ===");
    }
    
    Serial.println("\nPress any key to flash again...");
  }
}
