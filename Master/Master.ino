/*
 * 24Clocks Master Firmware
 * 
 * Controls two ATmega2560 slaves via Serial to drive 24 analog clocks.
 * 
 */

#include <Arduino.h>
#include <serialLink.h>
#include <ezTime.h>
#include <ArduinoOTA.h>
#include <DualLogger.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include "CommonConfig.h"

HardwareSerial SerialSlave1(SLAVE1_UART_NUM);
HardwareSerial SerialSlave2(SLAVE2_UART_NUM);

SerialLink slave1(SerialSlave1);
SerialLink slave2(SerialSlave2);

DualLogger logger;
WiFiUDP discoveryUdp;

// TODO: Handle mode change
bool ntpMode = false;
bool ntpSyncFailed = false;

unsigned long timeOffsetMillis = 0;
uint32_t secondsSinceMidnight = 0;
uint16_t minutesSinceMidnight = 0;

const uint32_t oneDaySeconds = 24*60*60;
const unsigned long oneDayMillis = (unsigned long)oneDaySeconds*1000UL;

void sendCommandToSlaves(const char* command);
void sendCommandToSpecificSlave(int slaveNum, const char* command);
void handleSlaveMessage1(const char* rawCommand);
void handleSlaveMessage2(const char* rawCommand);
void handleSlaveMessage(const char* rawCommand, int slaveNum);
void getTimeString(char* buffer);
void handleCommand();
void setTimeInSeconds(unsigned long secondsSinceMidnight);
unsigned long getTimeInSeconds();
void setDisplayTime(const char* time);
void setHome();
bool setNTP();
void handleDiscoveryProbe();

void setup() {

    Serial.begin(115200);
    delay(1);
    Serial.println("Launching Master");

    // WiFi Setup
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin(MASTER_MDNS_HOSTNAME)) {
        MDNS.addService("telnet", "tcp", MASTER_TCP_PORT);
        Serial.printf("mDNS started: %s.local\n", MASTER_MDNS_HOSTNAME);
    } else {
        Serial.println("mDNS failed to start");
    }

    if (discoveryUdp.begin(MASTER_DISCOVERY_PORT)) {
        Serial.printf("Discovery UDP listening on port %u\n", MASTER_DISCOVERY_PORT);
    } else {
        Serial.printf("Discovery UDP failed on port %u\n", MASTER_DISCOVERY_PORT);
    }

    // Start Logger (Telnet Server)
    logger.begin();
    logger.println("Master Ready. Telnet logging active.");

    SerialSlave1.setRxBufferSize(512);
    SerialSlave2.setRxBufferSize(512);
    SerialSlave1.begin(115200, SERIAL_8N1, SLAVE1_RX_PIN, SLAVE1_TX_PIN);
    SerialSlave2.begin(115200, SERIAL_8N1, SLAVE2_RX_PIN, SLAVE2_TX_PIN);

    slave1.setCommandCallback(handleSlaveMessage1);
    slave2.setCommandCallback(handleSlaveMessage2);

    // Set slave offsets
    sendCommandToSpecificSlave(1, "SETSLAVEOFFSET=0");
    sendCommandToSpecificSlave(2, "SETSLAVEOFFSET=1");
    delay(100); // Give slaves time to process the command

    ArduinoOTA.setHostname(MASTER_MDNS_HOSTNAME);

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
    
    setNTP();

}

void sendDebugStatus() {
    unsigned long uptime = millis() / 1000;
    char timeStr[5];
    getTimeString(timeStr);
    logger.println("STATUS uptime=" + String(uptime));
    logger.println("STATUS ntpMode=" + String(ntpMode));
    logger.println("STATUS timeOffsetMillis=" + String(timeOffsetMillis));
    logger.println("STATUS secondsSinceMidnight=" + String(secondsSinceMidnight));
    logger.println("STATUS minutesSinceMidnight=" + String(minutesSinceMidnight));
    logger.println("STATUS timeStr=" + String(timeStr));
}

void loop() {
    ArduinoOTA.handle();
    logger.handle(); // Handle Telnet clients
    handleDiscoveryProbe();

    if (logger.consumeNewClient()) {
        sendDebugStatus();
    }

    slave1.loop();
    slave2.loop();

    // Update every minute
    unsigned int newMinutesSinceMidnight = getTimeInSeconds() / 60;
    if (ntpMode && newMinutesSinceMidnight != minutesSinceMidnight) {
        // Resync NTP: every hour normally, every minute if last sync failed
        bool shouldResync = (newMinutesSinceMidnight % 60 == 0) || ntpSyncFailed;
        if (shouldResync) {
            logger.println("LOG NTP resync...");
            if (setNTP()) {
                ntpSyncFailed = false;
            } else {
                ntpSyncFailed = true;
                // NTP failed, move hands using internal clock
                minutesSinceMidnight = newMinutesSinceMidnight;
                char timeStr[5];
                getTimeString(timeStr);
                setDisplayTime(timeStr);
            }
        } else {
            minutesSinceMidnight = newMinutesSinceMidnight;
            char timeStr[5];
            getTimeString(timeStr);
            setDisplayTime(timeStr);
        }
    }

    handleCommand();

}

void handleSlaveMessage1(const char* rawCommand) {
    //logger.println("handleSlaveMessage1");
    handleSlaveMessage(rawCommand, 1);
}

void handleSlaveMessage2(const char* rawCommand) {
    //logger.println("handleSlaveMessage2");
    handleSlaveMessage(rawCommand, 2);
}

void handleSlaveMessage(const char* rawCommand, int slaveNum) {
    String msg(rawCommand);
    // If it's a POS message, emit structured POS line
    if (msg.startsWith("POS=")) {
        logger.printf("POS %d %s\n", slaveNum, msg.substring(4).c_str());
    } else {
        logger.printf("LOG [SLAVE %d] %s\n", slaveNum, rawCommand);
    }
}

void sendCommandToSlaves(const char* command) {
    slave1.sendCommand("CMD", command);
    slave2.sendCommand("CMD", command);
}

void sendCommandToSpecificSlave(int slaveNum, const char* command) {
    if (slaveNum == 1) {
        slave1.sendCommand("CMD", command);
    } else if (slaveNum == 2) {
        slave2.sendCommand("CMD", command);
    }
}

void getTimeString(char* buffer) {

    unsigned int minutes = minutesSinceMidnight%60;
    unsigned int hours = (minutesSinceMidnight-minutes)/60;

    logger.printf("LOG newTime %d --> %02d%02d\n", minutesSinceMidnight, hours, minutes);
    sprintf(buffer, "%02d%02d", hours, minutes);
}

void handleCommand() {

    String command = "";

    // Check Serial first
    if (Serial.available() > 0) {
        command = Serial.readStringUntil('\n');
    } 
    // Then check Telnet
    else if (logger.available() > 0) {
        command = logger.readStringUntil('\n');
    }

    command.trim();
    if (command == "") return;

    logger.println("LOG Command received: " + command);

    // Split command into name and parameter
    String cmdName = command;
    String cmdParam = "";
    int eqPos = command.indexOf('=');
    if (eqPos != -1) {
        cmdName = command.substring(0, eqPos);
        cmdParam = command.substring(eqPos + 1);
    }

    // Match commands by exact name
    if (cmdName == "FINETUNE") {
        // Format: FINETUNE=R,C,L,D (e.g. FINETUNE=0,1,0,+5.00)
        // R=row, C=col, L=hand(0/1), D=delta degrees
        int c1 = cmdParam.indexOf(',');
        int c2 = cmdParam.indexOf(',', c1 + 1);
        int c3 = cmdParam.indexOf(',', c2 + 1);
        if (c1 == -1 || c2 == -1 || c3 == -1) {
            logger.println("ERR FINETUNE invalid format (use R,C,L,D)");
            return;
        }
        int R = cmdParam.substring(0, c1).toInt();
        int C = cmdParam.substring(c1 + 1, c2).toInt();
        int L = cmdParam.substring(c2 + 1, c3).toInt();
        float D = cmdParam.substring(c3 + 1).toFloat();

        int slaveNum = (C < 2) ? 1 : 2;
        int boardIdx = R;
        int motorIdx = (C % 2) * 2 + L;

        char cmdBuffer[32];
        sprintf(cmdBuffer, "FINETUNE=%d,%d,%.2f", boardIdx, motorIdx, D);

        sendCommandToSpecificSlave(slaveNum, cmdBuffer);
        sendCommandToSlaves("GETPOS");
        logger.println("OK FINETUNE");

    } else if (cmdName == "SETTIME") {
        if (cmdParam.length() < 4) {
            logger.println("ERR SETTIME invalid parameter");
            return;
        }
        unsigned int tempIntTime = atoi(cmdParam.c_str());
        unsigned int minutes = tempIntTime % 100;
        unsigned int hours = (tempIntTime - minutes) / 100;
        unsigned long newTimeInSeconds = minutes * 60 + hours * 60 * 60;

        setTimeInSeconds(newTimeInSeconds);
        ntpMode = false;
        minutesSinceMidnight = 9999; // Force update
        logger.println("OK SETTIME");

    } else if (cmdName == "SETNTP") {
        setNTP();
        logger.println("OK SETNTP");

    } else if (cmdName == "RESETHOME") {
        sendCommandToSlaves(command.c_str());
        sendCommandToSlaves("GETPOS");
        logger.println("OK RESETHOME");

    } else if (cmdName == "SETHOME") {
        ntpMode = false;
        setHome();
        logger.println("OK SETHOME");

    } else if (cmdName == "SETZERO") {
        ntpMode = false;
        setDisplayTime("0000");
        logger.println("OK SETZERO");

    } else if (cmdName == "SETLED") {
        sendCommandToSlaves(command.c_str());
        logger.println("OK SETLED");

    } else if (cmdName == "ECHO") {
        logger.println("OK ECHO");

    } else if (cmdName == "QUIT") {
        logger.println("OK QUIT");
        logger.disconnectClient();

    } else if (cmdName == "UPTIME") {
        unsigned long uptime = millis() / 1000;
        logger.println("STATUS uptime=" + String(uptime));
        logger.println("OK UPTIME");

    } else if (cmdName == "DEBUG") {
        unsigned long uptime = millis() / 1000;
        char timeStr[5];
        getTimeString(timeStr);
        logger.println("STATUS uptime=" + String(uptime));
        logger.println("STATUS ntpMode=" + String(ntpMode));
        logger.println("STATUS timeOffsetMillis=" + String(timeOffsetMillis));
        logger.println("STATUS secondsSinceMidnight=" + String(secondsSinceMidnight));
        logger.println("STATUS minutesSinceMidnight=" + String(minutesSinceMidnight));
        logger.println("STATUS timeStr=" + String(timeStr));
        logger.println("OK DEBUG");

    } else if (cmdName == "GETPOS") {
        sendCommandToSlaves("GETPOS");
        logger.println("OK GETPOS");

    } else {
        logger.println("ERR UNKNOWN " + command);
    }

}

void setTimeInSeconds(unsigned long secondsSinceMidnight) {
    // timeOffsetMillis is the difference between the desired time and millis(),
    // wrapped to stay within one day (in milliseconds).
    unsigned long targetMillis = secondsSinceMidnight * 1000UL;
    unsigned long currentMillisOfDay = millis() % oneDayMillis;
    if (targetMillis >= currentMillisOfDay) {
        timeOffsetMillis = targetMillis - currentMillisOfDay;
    } else {
        timeOffsetMillis = targetMillis - currentMillisOfDay + oneDayMillis;
    }
}

unsigned long getTimeInSeconds() {
    // The returned value must be between 0 and oneDaySeconds-1
    return ((timeOffsetMillis + millis()) % oneDayMillis) / 1000UL;
}

void setDisplayTime(const char* time) {

    logger.printf("LOG setDisplayTime: %s\n", time);

    char buffer [13];
    sprintf(buffer, "SETTIME=%s\0", time);
    sendCommandToSlaves(buffer);
    sendCommandToSlaves("GETPOS");
    
}

void setHome() {
    logger.println("LOG Going home...");
    sendCommandToSlaves("SETHOME");
    sendCommandToSlaves("GETPOS");
}

bool setNTP() {

    if (waitForSync(30)) {
        Timezone timezone;
        timezone.setLocation("Europe/Rome");

        // If timezone rules failed to load, fall back to hardcoded POSIX string
        // CET-1CEST,M3.5.0,M10.5.0/3 = Rome: UTC+1, DST UTC+2
        if (timezone.getPosix().isEmpty()) {
            logger.println("LOG Timezone rules not loaded, using hardcoded POSIX");
            timezone.setPosix("CET-1CEST,M3.5.0,M10.5.0/3");
        }

        setTimeInSeconds(timezone.hour()*60*60 + timezone.minute()*60 + timezone.second());
        logger.println("LOG NTP time: " + timezone.dateTime());

        ntpMode = true;

        // Force immediate display update
        minutesSinceMidnight = getTimeInSeconds() / 60;
        char timeStr[5];
        getTimeString(timeStr);
        setDisplayTime(timeStr);
        return true;
    } else {
        logger.println("LOG NTP sync timed out");
        return false;
    }

}

void handleDiscoveryProbe() {
    int packetSize = discoveryUdp.parsePacket();
    if (packetSize <= 0) return;

    char buffer[96];
    int len = discoveryUdp.read(buffer, sizeof(buffer) - 1);
    if (len <= 0) return;
    buffer[len] = '\0';

    if (strcmp(buffer, MASTER_DISCOVERY_REQUEST) != 0) return;

    IPAddress remoteIp = discoveryUdp.remoteIP();
    uint16_t remotePort = discoveryUdp.remotePort();
    String localIp = WiFi.localIP().toString();

    char response[160];
    snprintf(
        response,
        sizeof(response),
        "%s;IP=%s;PORT=%u;HOST=ESP32-Master",
        MASTER_DISCOVERY_RESPONSE_PREFIX,
        localIp.c_str(),
        MASTER_TCP_PORT
    );

    discoveryUdp.beginPacket(remoteIp, remotePort);
    discoveryUdp.write((const uint8_t*)response, strlen(response));
    discoveryUdp.endPacket();
}
