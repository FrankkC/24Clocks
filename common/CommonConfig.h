#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

// WiFi Configuration
// To override these values locally, create CommonConfig.local.h (not tracked by git), e.g.:
//   #undef WIFI_SSID
//   #define WIFI_SSID "MyNetwork"
//   #undef WIFI_PASSWORD
//   #define WIFI_PASSWORD "mypassword"
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// Master network ports and discovery protocol
#define MASTER_TCP_PORT 23
#define MASTER_DISCOVERY_PORT 42124
#define MASTER_DISCOVERY_REQUEST "24CLOCKS_DISCOVER_V1"
#define MASTER_DISCOVERY_RESPONSE_PREFIX "24CLOCKS_MASTER_V1"

// Load local overrides if present (not tracked by git)
#if __has_include("CommonConfig.local.h")
#include "CommonConfig.local.h"
#endif

// Slave 1 Configuration
#define SLAVE1_TX_PIN 32
#define SLAVE1_RX_PIN 33
#define SLAVE1_RESET_PIN 25
#define SLAVE1_UART_NUM 1

// Slave 2 Configuration
#define SLAVE2_TX_PIN 22
#define SLAVE2_RX_PIN 23
#define SLAVE2_RESET_PIN 26
#define SLAVE2_UART_NUM 2

#endif
