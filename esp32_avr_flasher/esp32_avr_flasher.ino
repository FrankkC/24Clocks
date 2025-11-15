#include "avr_flash_arduino.h"
#include "firmware_slave.h"

// Definisce quale porta seriale hardware usare (es. Serial2)
#define AVR_SERIAL Serial2

void setup() {
  // Seriale per il monitor di debug
  Serial.begin(115200); 
  
  // Configura l'hardware UART2 sui pin corretti.
  // Questa operazione è FONDAMENTALE e va fatta qui.
  // L'inizializzazione del driver UART viene eseguita in `avr_flash_arduino.cpp`
  // tramite `init_avr_uart()`; non è necessario chiamare Serial2.begin() qui.
  // Se vuoi usare Serial2 separatamente per debug, inizializzalo qui con pin diversi.

  Serial.println("ESP32 ready. Press a key in the Serial Monitor to start flashing AVR.");
}

void loop() {
  if (Serial.available()) {
    Serial.read(); // Pulisce il buffer del monitor

    Serial.println("Starting AVR flash process...");

    // Avvia il processo di flashing.
    if (flash_avr_firmware(firmware_slave_hex)) {
      Serial.println("-------------------------");
      Serial.println("AVR flashed successfully!");
      Serial.println("-------------------------");
    } else {
      Serial.println("---------------------");
      Serial.println("Failed to flash AVR.");
      Serial.println("---------------------");
    }
  }
}
