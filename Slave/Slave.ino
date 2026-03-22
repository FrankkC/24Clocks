/*
 * 24Clocks Slave Firmware
 * 
 * Controls 12 Switec X12 motors (24 hands) via custom driver.
 * Receives commands from Master via Serial.
 * 
 */

#include <Arduino.h>
#include "SwitecX12.h"
#include "ClockPositions.h"
#include "ClockPins.h"
#include <serialLink.h>
#include <EEPROM.h>

#define MASTER Serial

constexpr uint16_t STEPS = 360 * 12;
constexpr uint8_t RESETPIN = 17;


constexpr uint8_t CONNECTED_BOARDS = 6;

SerialLink masterLink(MASTER);

// Must be set to 0 for the left slave and 1 for the right slave
uint8_t slaveOffset = 255; // Default to an invalid value, will be set by Master

SwitecX12 boards[CONNECTED_BOARDS];

struct MotorPositions {
    uint32_t sequence;
    int positions[CONNECTED_BOARDS * 4];
    uint8_t crc;
};

uint32_t currentSequence = 0;
int lastSavedPositions[CONNECTED_BOARDS * 4];
int eepromMaxEntries = 0;

void addBoard(char boardIndex);
void handleCommand(const char* rawCommand);
void setLocalDisplayTime(const char* time);
void setLocalHome();
void setLocalCurrentTime(const char* time);
void loadPositions();
void checkAndSavePositions();

void setup() {

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    delay(1);

    pinMode(RESETPIN, OUTPUT);
    digitalWrite(RESETPIN, HIGH);

    /*2-13*/
    for (int i = 2; i < 14; i++) {
        pinMode(i, OUTPUT);
        digitalWrite(i, LOW);
    }
    /*18-53*/
    for (int i = 18; i < 54; i++) {
        pinMode(i, OUTPUT);
        digitalWrite(i, LOW);
    }

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        addBoard(i);
    }

    MASTER.begin(115200);
    delay(1);

    masterLink.setCommandCallback(handleCommand);

    masterLink.sendCommand("MSG_","Launching Slave");

    eepromMaxEntries = EEPROM.length() / sizeof(MotorPositions);
    loadPositions();

}

void loop() {

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        boards[i].update();
    }

    masterLink.loop();

    checkAndSavePositions();

}

void addBoard(char boardIndex) {

    unsigned char pinStep[4];
    unsigned char pinDir[4];
    boolean reversed[4];

    for (int i=0; i<4; i++) {
        pinDir[i] = pins[boardIndex * 4 + i][0];
        pinStep[i] = pins[boardIndex * 4 + i][1];
        reversed[i] = pins[boardIndex * 4 + i][2];
    }
    
    boards[boardIndex] = SwitecX12(STEPS, pinStep, pinDir, reversed);

}

void handleCommand(const char* rawCommand) {

    masterLink.sendCommand("MSG_",rawCommand);

    String command = String(rawCommand);

    if (command.startsWith("CMD")) {
        String cmd = command.substring(3);
        if (cmd.startsWith("SETTIME=")) {
            String newTime = cmd.substring(8);
            setLocalDisplayTime(newTime.c_str());
        } else if (cmd.startsWith("SETHOME")) {
            setLocalHome();
        } else if (cmd.startsWith("SETSLAVEOFFSET=")) {
            String offsetStr = cmd.substring(15);
            uint8_t newOffset = offsetStr.toInt();
            if (newOffset == 0 || newOffset == 1) {
                slaveOffset = newOffset;
                masterLink.sendCommand("MSG_","slaveOffset: "+slaveOffset);
            } else {
                masterLink.sendCommand("MSG_","Invalid slave offset received");
            }
        } else if (cmd.startsWith("SETLED=1")) {
            digitalWrite(LED_BUILTIN, HIGH);
        } else if (cmd.startsWith("SETLED=0")) {
            digitalWrite(LED_BUILTIN, LOW);
        } else if (cmd.startsWith("FINETUNE=")) {
            // Expected format: FINETUNE=B,M,D
            // e.g. FINETUNE=0,2,3.5
            String params = cmd.substring(9);
            int firstComma = params.indexOf(',');
            int secondComma = params.indexOf(',', firstComma + 1);
            
            if (firstComma > 0 && secondComma > 0) {
                int boardIdx = params.substring(0, firstComma).toInt();
                int motorIdx = params.substring(firstComma + 1, secondComma).toInt();
                float degrees = params.substring(secondComma + 1).toFloat();
                
                if (boardIdx >= 0 && boardIdx < CONNECTED_BOARDS && motorIdx >= 0 && motorIdx < 4) {
                    boards[boardIdx].fineTune(motorIdx, degrees);
                    masterLink.sendCommand("MSG_", "Fine tune executed");
                }
            }
        } else if (cmd.startsWith("RESETHOME=")) {
            String newTime = cmd.substring(10);
            setLocalCurrentTime(newTime.c_str());
        } else if (cmd.startsWith("GETPOS")) {
            String posStr = "";
            posStr.reserve(200);
            for (int i = 0; i < CONNECTED_BOARDS; i++) {
                for (int m = 0; m < 4; m++) {
                    if (i > 0 || m > 0) posStr += ",";
                    posStr += String(boards[i].getCurrentRotation(m), 2);
                }
            }
            masterLink.sendCommand("POS=", posStr.c_str());
        }
    }
}

void setLocalDisplayTime(const char* time) {
    if (slaveOffset != 0 && slaveOffset != 1) {
        return;
    }
    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        int timeDigit = (i/3) * 2 + slaveOffset;

        boards[i].setTargetRotation(0, numbers[time[timeDigit] - '0'][i%3][0][0]);   // Left hour hand
        boards[i].setTargetRotation(1, numbers[time[timeDigit] - '0'][i%3][0][1]);   // Left minutes hand
        boards[i].setTargetRotation(2, numbers[time[timeDigit] - '0'][i%3][1][0]);   // Right hour hand
        boards[i].setTargetRotation(3, numbers[time[timeDigit] - '0'][i%3][1][1]);   // Right minutes hand

    }
}

void setLocalHome() {
    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        boards[i].setTargetRotation(0, 0);   // Left hour hand
        boards[i].setTargetRotation(1, 0);   // Left minutes hand
        boards[i].setTargetRotation(2, 0);   // Right hour hand
        boards[i].setTargetRotation(3, 0);   // Right minutes hand

    }
}

void setLocalCurrentTime(const char* time) {
    if (slaveOffset != 0 && slaveOffset != 1) {
        return;
    }
    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        int timeDigit = (i/3) * 2 + slaveOffset;

        // Set current position (not target) to match the displayed time
        // This tells the motor "you are currently at this position"
        boards[i].setCurrentPosition(0, boards[i].rotationToSteps(numbers[time[timeDigit] - '0'][i%3][0][0]));   // Left hour hand
        boards[i].setCurrentPosition(1, boards[i].rotationToSteps(numbers[time[timeDigit] - '0'][i%3][0][1]));   // Left minutes hand
        boards[i].setCurrentPosition(2, boards[i].rotationToSteps(numbers[time[timeDigit] - '0'][i%3][1][0]));   // Right hour hand
        boards[i].setCurrentPosition(3, boards[i].rotationToSteps(numbers[time[timeDigit] - '0'][i%3][1][1]));   // Right minutes hand

    }
}

void loadPositions() {
    uint32_t maxSeq = 0;
    int bestSlot = -1;
    
    for (int i = 0; i < eepromMaxEntries; i++) {
        MotorPositions entry;
        EEPROM.get(i * sizeof(MotorPositions), entry);
        
        // Calculate CRC
        uint8_t calcCrc = 0;
        const uint8_t* p = (const uint8_t*)&entry;
        for (size_t j = 0; j < sizeof(MotorPositions) - 1; j++) {
            calcCrc ^= p[j];
        }
        
        if (calcCrc == entry.crc) {
            if (entry.sequence >= maxSeq) {
                maxSeq = entry.sequence;
                bestSlot = i;
            }
        }
    }
    
    if (bestSlot >= 0) {
        MotorPositions entry;
        EEPROM.get(bestSlot * sizeof(MotorPositions), entry);
        currentSequence = entry.sequence;
        
        for (int i = 0; i < CONNECTED_BOARDS; i++) {
            for (int m = 0; m < 4; m++) {
                int pos = entry.positions[i*4 + m];
                boards[i].setCurrentPosition(m, pos);
                lastSavedPositions[i*4 + m] = pos;
            }
        }
        masterLink.sendCommand("MSG_", "Positions loaded from EEPROM");
    } else {
        masterLink.sendCommand("MSG_", "No valid positions in EEPROM");
    }
}

void checkAndSavePositions() {
    bool allStopped = true;
    bool changed = false;
    
    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        if (!boards[i].allStopped()) {
            allStopped = false;
            break;
        }
    }
    
    if (allStopped) {
        for (int i = 0; i < CONNECTED_BOARDS; i++) {
            for (int m = 0; m < 4; m++) {
                if (boards[i].currentStep[m] != lastSavedPositions[i*4 + m]) {
                    changed = true;
                    break;
                }
            }
            if (changed) break;
        }
    }
    
    if (allStopped && changed) {
        // Save
        currentSequence++;
        MotorPositions entry;
        entry.sequence = currentSequence;
        
        for (int i = 0; i < CONNECTED_BOARDS; i++) {
            for (int m = 0; m < 4; m++) {
                entry.positions[i*4 + m] = boards[i].currentStep[m];
                lastSavedPositions[i*4 + m] = boards[i].currentStep[m];
            }
        }
        
        // Calculate CRC
        entry.crc = 0;
        const uint8_t* p = (const uint8_t*)&entry;
        for (size_t j = 0; j < sizeof(MotorPositions) - 1; j++) {
            entry.crc ^= p[j];
        }
        
        int slot = currentSequence % eepromMaxEntries;
        EEPROM.put(slot * sizeof(MotorPositions), entry);
        
        // masterLink.sendCommand("MSG_", "Positions saved to EEPROM");
    }
}


