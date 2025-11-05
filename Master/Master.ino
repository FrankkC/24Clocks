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
#include "AVRFlasher.h"
#include "firmware_slave.h"

// -- Flasher Configuration --
#define SLAVE1_RESET_PIN 25
#define SLAVE2_RESET_PIN 26


// -- End of Flasher Configuration --


#define RXD1 18
#define TXD1 19
#define RXD2 16
#define TXD2 17

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

void flashSlave(int slaveNum);
void readAndPrintSlaveSerial(HardwareSerial& slaveSerial, int slaveNum);
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

    pinMode(SLAVE1_RESET_PIN, OUTPUT);
    pinMode(SLAVE2_RESET_PIN, OUTPUT);
    digitalWrite(SLAVE1_RESET_PIN, HIGH);
    digitalWrite(SLAVE2_RESET_PIN, HIGH);

    SerialSlave1.begin(115200, SERIAL_8N1, RXD1, TXD1);
    SerialSlave2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    // Set slave offsets
    sendCommandToSpecificSlave(1, "SETSLAVEOFFSET=0");
    sendCommandToSpecificSlave(2, "SETSLAVEOFFSET=1");
    delay(100); // Give slaves time to process the command

    WifiManager::init([]() {
        setNTP();
    });

}

void loop() {

    slave1.loop();
    slave2.loop();

    readAndPrintSlaveSerial(SerialSlave1, 1);
    readAndPrintSlaveSerial(SerialSlave2, 2);

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


void flashSlave(int slaveNum) {
    Serial.printf("--- Starting flash process for slave %d ---\n", slaveNum);

    HardwareSerial* slaveSerial = nullptr;
    int resetPin = -1;
    
    if (slaveNum == 1) {
        // IMPORTANT: End the serial port to release it from SerialLink before flashing
        Serial.println("Pausing communication on SerialSlave1...");
        SerialSlave1.end();
        slaveSerial = &SerialSlave1;
        resetPin = SLAVE1_RESET_PIN;
    } else if (slaveNum == 2) {
        // IMPORTANT: End the serial port to release it from SerialLink before flashing
        Serial.println("Pausing communication on SerialSlave2...");
        SerialSlave2.end();
        slaveSerial = &SerialSlave2;
        resetPin = SLAVE2_RESET_PIN;
    }

    if (slaveSerial && resetPin != -1) {
        const char* firmware_data = firmware_slave_hex;
        size_t firmware_size = sizeof(firmware_slave_hex);

        AVRFlasher flasher(*slaveSerial, resetPin);
        bool success = flasher.flash((const uint8_t*)firmware_data, firmware_size);

        if (success) {
            Serial.printf("--- Flash for slave %d successful ---\n", slaveNum);
        } else {
            Serial.printf("--- Flash for slave %d failed ---\n", slaveNum);
        }
    }

    // Re-initialize the serial port for normal communication
    if (slaveNum == 1) {
        Serial.println("Resuming communication on SerialSlave1...");
        SerialSlave1.begin(115200, SERIAL_8N1, RXD1, TXD1);
    } else if (slaveNum == 2) {
        Serial.println("Resuming communication on SerialSlave2...");
        SerialSlave2.begin(115200, SERIAL_8N1, RXD2, TXD2);
    }
}

void readAndPrintSlaveSerial(HardwareSerial& slaveSerial, int slaveNum) {
    while (slaveSerial.available()) {
        Serial.printf("[SLAVE%d] %c", slaveNum, (char)slaveSerial.read());
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
            setDisplayTime(newTime.c_str());

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
        } else if (command.startsWith("FLASH")) {
            int slaveNum = command.substring(6).toInt();
            if (slaveNum == 1 || slaveNum == 2) {
                Serial.printf("Received WiFi flash command for slave %d\n", slaveNum);
                flashSlave(slaveNum);
                // flashSlave prints its own success/failure messages to Serial
                // We can add a WifiManager::sendData here if needed, but Serial is more verbose for flashing
                WifiManager::sendData("FLASH COMMAND SENT");
            } else {
                WifiManager::sendData("Invalid slave number. Use 1 or 2.");
            }
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
