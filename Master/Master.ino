/*
MASTER:
Serial1 --> Slave 1
Serial2 --> Slave 2
SerialZ --> Serial Monitor

SLAVE 1:
Serial --> Serial Monitor
Serial3 --> Master

SLAVE 2:
Serial --> Serial Monitor
Serial3 --> Master
*/

/*
TODO:
Step3: Functionality to adjust the position of the hands using telnet or the Android app
*/

#include <Arduino.h>
#include "wifiManager.h"
#include <serialLink.h>
#include <ezTime.h>
#include <ArduinoOTA.h>


#define RXD1 32
#define TXD1 33
#define RXD2 35
#define TXD2 34

HardwareSerial SerialSlave1(1);
HardwareSerial SerialSlave2(2);

SerialLink slave1(SerialSlave1);
SerialLink slave2(SerialSlave2);

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
void getTimeString(char* buffer);
void handleWifiCommand();
void setTimeInSeconds(unsigned long secondsSinceMidnight);
unsigned long getTimeInSeconds();
void setDisplayTime(const char* time);
void setHome();
void setNTP();

void setup() {

    Serial.begin(115200);
    delay(1);
    Serial.println("Launching Master");

    SerialSlave1.begin(115200, SERIAL_8N1, RXD1, TXD1);
    SerialSlave2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    slave1.setCommandCallback(handleSlaveMessage1);
    slave2.setCommandCallback(handleSlaveMessage2);

    // Set slave offsets
    sendCommandToSpecificSlave(1, "SETSLAVEOFFSET=0");
    sendCommandToSpecificSlave(2, "SETSLAVEOFFSET=1");
    delay(100); // Give slaves time to process the command

    WifiManager::init([]() {
        ArduinoOTA.setHostname("ESP32-Master");

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
        
        setNTP();
    });

}

void loop() {
    ArduinoOTA.handle();
    slave1.loop();
    slave2.loop();

    /*if (!countMode && millis() > 10000) {
        countMode = true;
    }*/

    /*if (countMode && timer != (millis()/1000)%10) {
        timer = (millis()/1000)%10;
        char time[5];
        sprintf (time, "%d%d%d%d\0", timer, timer, timer, timer);
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

    handleWifiCommand();

}

void handleSlaveMessage1(const char* rawCommand) {
    Serial.println("handleSlaveMessage1");
    handleSlaveMessage(rawCommand, 1);
}

void handleSlaveMessage2(const char* rawCommand) {
    Serial.println("handleSlaveMessage2");
    handleSlaveMessage(rawCommand, 2);
}

void handleSlaveMessage(const char* rawCommand, int slaveNum) {
    Serial.printf("[SLAVE %d] %s\n", slaveNum, rawCommand);
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

    Serial.printf("newTime %d --> %02d%02d\n", minutesSinceMidnight, hours, minutes);
    sprintf(buffer, "%02d%02d", hours, minutes);
}

void handleWifiCommand() {

    String command = WifiManager::readCommand();

    if (command != "") {

        Serial.println("handleWifiCommand: " + command);

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

            WifiManager::sendData("SET TIME OK");
        } else if (command.indexOf("SETNTP") != -1) {
            setNTP();
            WifiManager::sendData("SETNTP OK");
        } else if (command.indexOf("SETHOME") != -1) {
            timeMode = false;
            countMode = false;
            setHome();
            WifiManager::sendData("SET HOME OK");
        } else if (command.indexOf("SETZERO") != -1) {
            timeMode = false;
            countMode = false;
            setDisplayTime("0000");
            WifiManager::sendData("SET ZERO OK");
        } else if (command.indexOf("SETCOUNT=1") != -1) {
            timeMode = false;
            countMode = true;
            WifiManager::sendData("SET COUNT MODE ON OK");
        } else if (command.indexOf("SETCOUNT=0") != -1) {
            countMode = false;
            WifiManager::sendData("SET COUNT MODE OFF OK");
        } else if (command.indexOf("SETMIN=0") != -1) {
            sendCommandToSlaves("SETMIN=0");
            WifiManager::sendData("SETMIN=0 OK");
        } else if (command.indexOf("SETMIN=1") != -1) {
            sendCommandToSlaves("SETMIN=1");
            WifiManager::sendData("SETMIN=1 OK");
        } else if (command.indexOf("SETHOU=0") != -1) {
            sendCommandToSlaves("SETHOU=0");
            WifiManager::sendData("SETHOU=0 OK");
        } else if (command.indexOf("SETHOU=1") != -1) {
            sendCommandToSlaves("SETHOU=1");
            WifiManager::sendData("SETHOU=1 OK");
        } else if (command.indexOf("SETSPIN=") != -1) {
            // TODO: Deactivate timeMode and countMode
            sendCommandToSlaves(command.c_str());
            WifiManager::sendData("SET SPIN OK");
        } else if (command.indexOf("ECHO") != -1) {
            WifiManager::sendData("ECHO OK");
        } else if (command.indexOf("UPTIME") != -1) {
            // Print device uptime in seconds
            unsigned long uptime = millis() / 1000;
            WifiManager::sendData("UPTIME=" + String(uptime) + " seconds");
        } else if (command.indexOf("DEBUG") != -1) {
            unsigned long uptime = millis() / 1000;
            WifiManager::sendData("UPTIME=" + String(uptime) + " seconds");
            WifiManager::sendData("countMode=" + String(countMode));
            WifiManager::sendData("timeMode=" + String(timeMode));
            WifiManager::sendData("timeOffset=" + String(timeOffset) + " seconds");
            WifiManager::sendData("secondsSinceMidnight=" + String(secondsSinceMidnight) + " seconds");
            WifiManager::sendData("minutesSinceMidnight=" + String(minutesSinceMidnight) + " minutes");
            char timeStr[5];
            getTimeString(timeStr);
            WifiManager::sendData("timeStr=" + String(timeStr));
        } else if (command.indexOf("SETLED") != -1) {
            sendCommandToSlaves(command.c_str());
            WifiManager::sendData("SETLED OK");
        } else {
            WifiManager::sendData("UNKNOWN COMMAND");
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

    Serial.printf("setDisplayTime: %s\n", time);

    char buffer [13];
    sprintf(buffer, "SETTIME=%s\0", time);
    sendCommandToSlaves(buffer);
    
}

void setHome() {
    Serial.println("Going home...");
    sendCommandToSlaves("SETHOME");
}

void setNTP() {

    if (waitForSync()) {
        Timezone timezone;
        timezone.setLocation("Europe/Rome");

        setTimeInSeconds(timezone.hour()*60*60 + timezone.minute()*60 + timezone.second());
        Serial.println("NTP time: " + timezone.dateTime());

        timeMode = true;
        countMode = false;
    } else {
        Serial.println("NTP time not available");
    }

}
