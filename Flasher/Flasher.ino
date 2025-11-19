/*
 * ESP32 AVR Flasher
 * Flashes firmware to two ATmega2560 slaves via STK500 protocol.
 * 
 */

#include "avr_flash_arduino.h"
#include "firmware_slave.h"
#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

// Configuration for Slave 1
const AVRFlashConfig slave1_config = {
  .reset_pin = 25,
  .tx_pin = 33,
  .rx_pin = 32,
  .uart_num = UART_NUM_1
};

// Configuration for Slave 2
const AVRFlashConfig slave2_config = {
  .reset_pin = 26,
  .tx_pin = 34,
  .rx_pin = 35,
  .uart_num = UART_NUM_2
};

bool flashing_done = false;

void setup() {
  Serial.begin(115200);
  
  // WiFi & OTA Setup
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.setHostname("ESP32-Flasher");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();
  
  // Additional delay to ensure connection is stable
  delay(500);
  
  Serial.println("\nESP32 Dual AVR Flasher Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting automatic flash in 2 seconds...");
  delay(2000);
}

void loop() {
  ArduinoOTA.handle();
  if (!flashing_done) {
    flashing_done = true;
    
    Serial.println("\n=== Starting Flash Process ===");
    
    // Flash Slave 1
    Serial.println("\n--- Flashing Slave 1 ---");
    Serial.println("Config: RESET=25, TX=33, RX=32, UART1");
    bool slave1_ok = flash_avr_firmware(firmware_slave_hex, slave1_config);
    
    if (slave1_ok) {
      Serial.println("=== Slave 1 Flash Successful ===");
    } else {
      Serial.println("=== Slave 1 Flash Failed ===");
    }
    
    delay(1000);
    
    // Flash Slave 2
    Serial.println("\n--- Flashing Slave 2 ---");
    Serial.println("Config: RESET=26, TX=34, RX=35, UART2");
    bool slave2_ok = flash_avr_firmware(firmware_slave_hex, slave2_config);
    
    if (slave2_ok) {
      Serial.println("=== Slave 2 Flash Successful ===");
    } else {
      Serial.println("=== Slave 2 Flash Failed ===");
    }
    
    // Summary
    Serial.println("\n=== Flash Summary ===");
    Serial.printf("Slave 1: %s\n", slave1_ok ? "SUCCESS" : "FAILED");
    Serial.printf("Slave 2: %s\n", slave2_ok ? "SUCCESS" : "FAILED");
    
    if (slave1_ok && slave2_ok) {
      Serial.println("\n=== Both Slaves Flashed Successfully ===");
    } else {
      Serial.println("\n=== Some Slaves Failed to Flash ===");
    }
    
    Serial.println("\nFlashing complete. Reset ESP32 to flash again.");
  }
}
