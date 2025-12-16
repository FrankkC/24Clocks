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
#include "CommonConfig.h"

HardwareSerial SerialSlave1(SLAVE1_UART_NUM);
HardwareSerial SerialSlave2(SLAVE2_UART_NUM);

SerialLink slave1(SerialSlave1);
SerialLink slave2(SerialSlave2);

DualLogger logger;

//int timer = 0;
bool countMode = false;
// TODO: Handle mode change
bool timeMode = false;

uint32_t timeOffset = 0;
uint32_t secondsSinceMidnight = 0;
uint16_t minutesSinceMidnight = 0;

const uint32_t oneDaySeconds = 24*60*60;
const uint32_t oneDayMillis = oneDaySeconds*1000;

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
void setNTP();

void setup() {

    Serial.begin(115200);
    delay(1);
    Serial.println("Launching Master");

    // WiFi Setup
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected to WiFi");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Start Logger (Telnet Server)
    logger.begin();
    logger.println("Master Ready. Telnet logging active.");

    SerialSlave1.begin(115200, SERIAL_8N1, SLAVE1_RX_PIN, SLAVE1_TX_PIN);
    SerialSlave2.begin(115200, SERIAL_8N1, SLAVE2_RX_PIN, SLAVE2_TX_PIN);

    slave1.setCommandCallback(handleSlaveMessage1);
    slave2.setCommandCallback(handleSlaveMessage2);

    // Set slave offsets
    sendCommandToSpecificSlave(1, "SETSLAVEOFFSET=0");
    sendCommandToSpecificSlave(2, "SETSLAVEOFFSET=1");
    delay(100); // Give slaves time to process the command

    ArduinoOTA.setHostname("ESP32-Master");

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

void loop() {
    ArduinoOTA.handle();
    logger.handle(); // Handle Telnet clients
    slave1.loop();
    slave2.loop();

    /*if (!countMode && millis() > 10000) {
        countMode = true;
    }*/

    /*if (countMode && timer != (millis()/1000)%10) {
        timer = (millis()/1000)%10;
        char time[5];
        sprintf (time, "%d%d%d%d\0", timer, timer, timer, timer, timer);
        setDisplayTime(time);
    }*/

    // Update every minute
    unsigned int newMinutesSinceMidnight = getTimeInSeconds() / 60;
    if (timeMode && newMinutesSinceMidnight != minutesSinceMidnight) {
        minutesSinceMidnight = newMinutesSinceMidnight;
        char timeStr[5];
        getTimeString(timeStr);
        setDisplayTime(timeStr);
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
    logger.printf("[SLAVE %d] %s\n", slaveNum, rawCommand);
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

    logger.printf("newTime %d --> %02d%02d\n", minutesSinceMidnight, hours, minutes);
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

    if (command != "") {
        command.trim(); // Remove whitespace/newlines

        if (command != "") {

            logger.println("Command received: " + command);

            if (command.indexOf("SETTIME=") != -1) {
                String newTime = command.substring(command.indexOf("TIME=") + 5, command.indexOf("TIME=") + 9);

                // Convert the time into seconds since midnight
                unsigned int tempIntTime = atoi(newTime.c_str());
                unsigned int minutes = tempIntTime % 100;
                unsigned int hours = (tempIntTime - minutes) / 100;

                // Invert the following lines if we are using the seconds count instead of the minutes
                unsigned long newTimeInSeconds = minutes * 60 + hours * 60 * 60;
                //unsigned long newTimeInSeconds = minutes + hours * 60;


                // Set timeoffset to this value
                setTimeInSeconds(newTimeInSeconds);

                // Set timeMode = true to start updating the hands position
                timeMode = true;
                countMode = false;

                logger.println("SET TIME OK");
            } else if (command.indexOf("SETNTP") != -1) {
                setNTP();
                logger.println("SETNTP OK");
            } else if (command.indexOf("RESETHOME=") != -1) {
                sendCommandToSlaves(command.c_str());
                logger.println("RESET HOME OK");
            } else if (command.indexOf("SETHOME") != -1) {
                timeMode = false;
                countMode = false;
                setHome();
                logger.println("SET HOME OK");
            } else if (command.indexOf("SETZERO") != -1) {
                timeMode = false;
                countMode = false;
                setDisplayTime("0000");
                logger.println("SET ZERO OK");
            } else if (command.length() >= 5 && isdigit(command.charAt(0)) && isdigit(command.charAt(1)) && isdigit(command.charAt(2)) && (command.charAt(3) == '+' || command.charAt(3) == '-')) {
                // RCL+D
                int R = command.substring(0, 1).toInt();
                int C = command.substring(1, 2).toInt();
                int L = command.substring(2, 3).toInt();
                float D = command.substring(3).toFloat();
                
                int slaveNum = (C < 2) ? 1 : 2;
                int boardIdx = R;
                int motorIdx = (C % 2) * 2 + L;
                
                char cmdBuffer[32];
                sprintf(cmdBuffer, "FINETUNE=%d,%d,%.2f", boardIdx, motorIdx, D);
                
                sendCommandToSpecificSlave(slaveNum, cmdBuffer);
                logger.println("FINETUNE SENT");
            } else if (command.indexOf("SETCOUNT=1") != -1) {
                timeMode = false;
                countMode = true;
                logger.println("SET COUNT MODE ON OK");
            } else if (command.indexOf("SETCOUNT=0") != -1) {
                countMode = false;
                logger.println("SET COUNT MODE OFF OK");
            } else if (command.indexOf("SETMIN=0") != -1) {
                sendCommandToSlaves("SETMIN=0");
                logger.println("SETMIN=0 OK");
            } else if (command.indexOf("SETMIN=1") != -1) {
                sendCommandToSlaves("SETMIN=1");
                logger.println("SETMIN=1 OK");
            } else if (command.indexOf("SETHOU=0") != -1) {
                sendCommandToSlaves("SETHOU=0");
                logger.println("SETHOU=0 OK");
            } else if (command.indexOf("SETHOU=1") != -1) {
                sendCommandToSlaves("SETHOU=1");
                logger.println("SETHOU=1 OK");
            } else if (command.indexOf("SETSPIN=") != -1) {
                // TODO: Deactivate timeMode and countMode
                sendCommandToSlaves(command.c_str());
                logger.println("SET SPIN OK");
            } else if (command.indexOf("ECHO") != -1) {
                logger.println("ECHO OK");
            } else if (command.indexOf("UPTIME") != -1) {
                // Print device uptime in seconds
                unsigned long uptime = millis() / 1000;
                logger.println("UPTIME=" + String(uptime) + " seconds");
            } else if (command.indexOf("DEBUG") != -1) {
                unsigned long uptime = millis() / 1000;
                logger.println("UPTIME=" + String(uptime) + " seconds");
                logger.println("countMode=" + String(countMode));
                logger.println("timeMode=" + String(timeMode));
                logger.println("timeOffset=" + String(timeOffset) + " seconds");
                logger.println("secondsSinceMidnight=" + String(secondsSinceMidnight) + " seconds");
                logger.println("minutesSinceMidnight=" + String(minutesSinceMidnight) + " minutes");
                char timeStr[5];
                getTimeString(timeStr);
                logger.println("timeStr=" + String(timeStr));
            } else if (command.indexOf("SETLED") != -1) {
                sendCommandToSlaves(command.c_str());
                logger.println("SETLED OK");
            } else {
                logger.println("UNKNOWN COMMAND");
            }

        }
    }

}

void setTimeInSeconds(unsigned long secondsSinceMidnight) {
    //assert(secondsSinceMidnight < oneDaySeconds);
    // timeOffset is the difference between the time we want to set and what is reported by millis().
    // the modulo 86400 is used to bring millis() back to a value within a day (if the device has been on for more than 24h)
    // if secondsSinceMidnight < oneDaySeconds, the result of timeOffset will always be between 0 and oneDaySeconds-1
    timeOffset = (secondsSinceMidnight - (millis()/1000)%oneDaySeconds) < 0 ?
        secondsSinceMidnight - (millis()/1000)%oneDaySeconds + oneDaySeconds :
        secondsSinceMidnight - (millis()/1000)%oneDaySeconds;

}

unsigned long getTimeInSeconds() {
    // The returned value must be between 0 and oneDaySeconds-1
    return (timeOffset + millis()/1000)%oneDaySeconds;
}

void setDisplayTime(const char* time) {

    logger.printf("setDisplayTime: %s\n", time);

    char buffer [13];
    sprintf(buffer, "SETTIME=%s\0", time);
    sendCommandToSlaves(buffer);
    
}

void setHome() {
    logger.println("Going home...");
    sendCommandToSlaves("SETHOME");
}

void setNTP() {

    if (waitForSync()) {
        Timezone timezone;
        timezone.setLocation("Europe/Rome");

        setTimeInSeconds(timezone.hour()*60*60 + timezone.minute()*60 + timezone.second());
        logger.println("NTP time: " + timezone.dateTime());

        timeMode = true;
        countMode = false;
    } else {
        logger.println("NTP time not available");
    }

}
