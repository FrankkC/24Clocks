/*
 * ESP32 AVR Flasher
 * Flashes firmware to two ATmega2560 slaves via STK500 protocol.
 * 
 */

#include "avr_flash_arduino.h"
#include "firmware_slave.h"
#include <DualLogger.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

DualLogger logger;

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
    .tx_pin = 22,
    .rx_pin = 23,
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

    // Start Telnet Server
    logger.begin();

    // Redirect AVR Flasher logs to DualLogger
    avr_debugger = &logger;

    ArduinoOTA.setHostname("ESP32-Flasher");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
            type = "sketch";
        else // U_SPIFFS
            type = "filesystem";
        logger.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        logger.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static unsigned int last_pct = 101;
        unsigned int pct = progress / (total / 100);
        if (pct != last_pct) {
            logger.printf("Progress: %u%%\n", pct);
            last_pct = pct;
        }
    });
    ArduinoOTA.onError([](ota_error_t error) {
        logger.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) logger.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) logger.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) logger.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) logger.println("Receive Failed");
        else if (error == OTA_END_ERROR) logger.println("End Failed");
    });

    ArduinoOTA.begin();

    // Additional delay to ensure connection is stable
    delay(500);

    logger.println("\nESP32 Dual AVR Flasher Ready");
    logger.print("IP address: ");
    logger.println(WiFi.localIP());
    logger.println("Telnet to port 23 to view logs.");
    logger.println("Starting automatic flash in 5 seconds (connect Telnet now)...");
    
    // Non-blocking delay to allow Telnet connection
    unsigned long start = millis();
    while (millis() - start < 5000) {
        logger.handle();
        delay(10);
    }
}

void loop() {
    ArduinoOTA.handle();
    logger.handle();

    if (!flashing_done) {
        flashing_done = true;

        logger.println("\n=== Starting Flash Process ===");

        // Flash Slave 1
        logger.println("\n--- Flashing Slave 1 ---");
        logger.println("Config: RESET=25, TX=33, RX=32, UART1");
        bool slave1_ok = flash_avr_firmware(firmware_slave_hex, slave1_config);

        if (slave1_ok) {
            logger.println("=== Slave 1 Flash Successful ===");
        } else {
            logger.println("=== Slave 1 Flash Failed ===");
        }

        delay(1000);

        // Flash Slave 2
        logger.println("\n--- Flashing Slave 2 ---");
        logger.println("Config: RESET=26, TX=22, RX=23, UART2");
        bool slave2_ok = flash_avr_firmware(firmware_slave_hex, slave2_config);

        if (slave2_ok) {
            logger.println("=== Slave 2 Flash Successful ===");
        } else {
            logger.println("=== Slave 2 Flash Failed ===");
        }

        // Summary
        logger.println("\n=== Flash Summary ===");
        logger.printf("Slave 1: %s\n", slave1_ok ? "SUCCESS" : "FAILED");
        logger.printf("Slave 2: %s\n", slave2_ok ? "SUCCESS" : "FAILED");

        if (slave1_ok && slave2_ok) {
            logger.println("\n=== Both Slaves Flashed Successfully ===");
        } else {
            logger.println("\n=== Some Slaves Failed to Flash ===");
        }

        logger.println("\nFlashing complete. Reset ESP32 to flash again.");
    }
}
