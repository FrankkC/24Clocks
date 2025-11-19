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

#define MASTER Serial

constexpr uint16_t STEPS = 360 * 12;
constexpr uint8_t RESETPIN = 17;


constexpr uint8_t CONNECTED_BOARDS = 6;

SerialLink masterLink(MASTER);

// Must be set to 0 for the left slave and 1 for the right slave
uint8_t slaveOffset = 255; // Default to an invalid value, will be set by Master

bool hoursHandsActive = true;
bool minutesHandsActive = true;

/*

Spin Mode

Bit 0: spin minutes hand
Bit 1: spin hours hand
Bit 2: cw/ccw (0/1) minutes hand direction
Bit 3: cw/ccw (0/1) hours hand direction

E.g.:
0000 --> spinMode = 0 --> no spin
0001 --> spinMode = 1 --> spin minutes hand clockwise
1111 --> spinMode = 15 --> spin both hands counter clockwise

*/
int spinMode = 0;

SwitecX12 boards[CONNECTED_BOARDS];

void addBoard(char boardIndex);
void handleCommand(const char* rawCommand);
void setLocalDisplayTime(const char* time);
void setLocalHome();

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

}

void loop() {

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        boards[i].update();
    }

    masterLink.loop();

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
        } else if (cmd.startsWith("SETMIN=0")) {
            minutesHandsActive = false;
        } else if (cmd.startsWith("SETMIN=1")) {
            minutesHandsActive = true;
        } else if (cmd.startsWith("SETHOU=0")) {
            hoursHandsActive = false;
        } else if (cmd.startsWith("SETHOU=1")) {
            hoursHandsActive = true;
        } else if (cmd.startsWith("SETSLAVEOFFSET=")) {
            String offsetStr = cmd.substring(15);
            uint8_t newOffset = offsetStr.toInt();
            if (newOffset == 0 || newOffset == 1) {
                slaveOffset = newOffset;
                masterLink.sendCommand("MSG_","slaveOffset: "+slaveOffset);
            } else {
                masterLink.sendCommand("MSG_","Invalid slave offset received");
            }
        } else if (cmd.startsWith("SETSPIN=")) {
            String spinCommand = cmd.substring(8);
            spinMode = ((spinCommand.charAt(0) == '1') << 3) | ((spinCommand.charAt(1) == '1') << 2) | ((spinCommand.charAt(2) == '1') << 1) | (spinCommand.charAt(3) == '1');
        } else if (cmd.startsWith("SETLED=1")) {
            digitalWrite(LED_BUILTIN, HIGH);
        } else if (cmd.startsWith("SETLED=0")) {
            digitalWrite(LED_BUILTIN, LOW);
        }
    }
}

void setLocalDisplayTime(const char* time) {
    if (slaveOffset != 0 && slaveOffset != 1) {
        return;
    }
    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        int timeDigit = (i/3) * 2 + slaveOffset;

        if (hoursHandsActive) boards[i].setTargetRotation(0, numbers[time[timeDigit] - '0'][i%3][0][0]);   // Left hour hand
        if (minutesHandsActive) boards[i].setTargetRotation(1, numbers[time[timeDigit] - '0'][i%3][0][1]);   // Left minutes hand
        if (hoursHandsActive) boards[i].setTargetRotation(2, numbers[time[timeDigit] - '0'][i%3][1][0]);   // Right hour hand
        if (minutesHandsActive) boards[i].setTargetRotation(3, numbers[time[timeDigit] - '0'][i%3][1][1]);   // Right minutes hand

    }
}

void setLocalHome() {
    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        if (hoursHandsActive) boards[i].setTargetRotation(0, 0);   // Left hour hand
        if (minutesHandsActive) boards[i].setTargetRotation(1, 0);   // Left minutes hand
        if (hoursHandsActive) boards[i].setTargetRotation(2, 0);   // Right hour hand
        if (minutesHandsActive) boards[i].setTargetRotation(3, 0);   // Right minutes hand

    }
}


