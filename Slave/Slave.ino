// Serial0 e Serial3 disponibili
// Serial1 occupata da collegamento board
// Serial2 RX occupato da pin reset

/*
SLAVE:
Serial --> Serial Monitor
Serial3 --> Master
*/

/*
TODO:
Step2: Fix bug Rotazione lancetta tra 2 e 3 alla sesta (settima) iterazione del counter
Step3: Funzionalità per regolare la posizione delle lancette e comunicazione con WiFi e App Android
*/

#include <Arduino.h>
#include "SwitecX12.h"
#include "ClockPositions.h"
#include "ClockPins.h"
#include "serialLink.h"

const int STEPS = 360 * 12;
const int RESETPIN = 17;


constexpr int CONNECTED_BOARDS = 6;

String commandBuffer;

// Should be set to 0 for the left slave and 1 for the right slave
int slaveOffset = 0;
bool hoursHandsActive = true;
bool minutesHandsActive = true;

SwitecX12 boards[CONNECTED_BOARDS];

void setup() {

    Serial.begin(115200);
    delay(1000);
    Serial.println("Launching Slave");

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

    SerialLink::init();

}

void loop() {

    bool allStopped = true;

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        boards[i].update();
        allStopped &= boards[i].allStopped();
    }

    handleMasterCommand();

}

void handleMasterCommand() {

    if (SerialLink::readCommand(commandBuffer)) {

        Serial.println("handleMasterCommand: " + commandBuffer);

        if (commandBuffer.startsWith("CMD")) {
            String command = commandBuffer.substring(3);
            if (command.startsWith("SETTIME=")) {
                String newTime = command.substring(8);
                setLocalDisplayTime(newTime.c_str());
            } else if (command.startsWith("SETHOME")) {
                setLocalHome();
            } else if (command.startsWith("SETMIN=0")) {
                minutesHandsActive = false;
            } else if (command.startsWith("SETMIN=1")) {
                minutesHandsActive = true;
            } else if (command.startsWith("SETHOU=0")) {
                hoursHandsActive = false;
            } else if (command.startsWith("SETHOU=1")) {
                hoursHandsActive = true;
            }
        }

        commandBuffer = "";

    }

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

void setLocalDisplayTime(const char* time) {
    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        int timeDigit = (i/3) * 2 + slaveOffset;

        if (hoursHandsActive) boards[i].setTargetRotation(0, numbers[time[timeDigit] - '0'][i%3][0][0]);   // Left hour hand
        if (minutesHandsActive) boards[i].setTargetRotation(1, numbers[time[timeDigit] - '0'][i%3][0][1]);   // Left minutes hand
        if (minutesHandsActive) boards[i].setTargetRotation(2, numbers[time[timeDigit] - '0'][i%3][1][0]);   // Right minutes hand
        if (hoursHandsActive) boards[i].setTargetRotation(3, numbers[time[timeDigit] - '0'][i%3][1][1]);   // Right hour hand

    }
}

void setLocalHome() {
    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        if (hoursHandsActive) boards[i].setTargetRotation(0, 0);   // Left hour hand
        if (minutesHandsActive) boards[i].setTargetRotation(1, 0);   // Left minutes hand
        if (minutesHandsActive) boards[i].setTargetRotation(2, 0);   // Right minutes hand
        if (hoursHandsActive) boards[i].setTargetRotation(3, 0);   // Right hour hand

    }
}


