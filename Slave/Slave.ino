// Serial0 e Serial3 disponibili
// Serial1 occupata da collegamento board
// Serial2 RX occupato da pin reset

/*
MASTER:
Serial --> WIFI
Serial3 --> Slave

SLAVE:
Serial --> Serial Monitor
Serial3 --> Master
*/

/*
TODO:
Step1: Collegamento MASTER<->SLAVE Usiamo Serial per comunicare con Slave e trasferiamo il serial monitor di Master a Slave
Step2: Fix bug Rotazione lancetta tra 2 e 3 alla sesta (settima) iterazione del counter
Step3: Funzionalità per regolare la posizione delle lancette e comunicazione con WiFi e App Android
*/

#include <Arduino.h>
#include "SwitecX12.h"
#include "ClockPositions.h"
#include "ClockPins.h"

const int STEPS = 360 * 12;
const int RESETPIN = 17;


constexpr int CONNECTED_BOARDS = 6;

String commandBuffer;
bool commandBufferReady = false;
int slaveOffset = 1;

#define SERIAL_MONITOR Serial
#define MASTER_SLAVE_SERIAL Serial3

SwitecX12 boards[CONNECTED_BOARDS];
int timer = 0;
bool countMode = false;

void setup() {

    MASTER_SLAVE_SERIAL.begin(115200);
    SERIAL_MONITOR.begin(115200);

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        addBoard(i);
    }
    
    digitalWrite(RESETPIN, HIGH);

}

void loop() {

    handleMasterCommand();

}

void handleMasterCommand() {

    while (MASTER_SLAVE_SERIAL.available()) {
        delay(1);
        if (MASTER_SLAVE_SERIAL.available() > 0) {
            char c = MASTER_SLAVE_SERIAL.read();
            if(c == ';') {
                commandBufferReady = true;
            } else {
                commandBuffer += c;
            }
        }
    }

    if (commandBufferReady) {
        serialMonitor(commandBuffer);
        commandBufferReady = false;
        commandBuffer = "";
    }

}

void serialMonitor(const String &s) {
    SERIAL_MONITOR.println(s);
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

void setDisplayTime(const char* time) {

    serialMonitor("setDisplayTime: " + String(time));

    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        int timeDigit = (i/3) * 2 + slaveOffset;

        boards[i].setTargetRotation(0, numbers[time[timeDigit] - '0'][i%3][0][0]);   // Left hour hand
        boards[i].setTargetRotation(1, numbers[time[timeDigit] - '0'][i%3][0][1]);   // Left minutes hand
        boards[i].setTargetRotation(2, numbers[time[timeDigit] - '0'][i%3][1][0]);   // Right hour hand
        boards[i].setTargetRotation(3, numbers[time[timeDigit] - '0'][i%3][1][1]);   // Right minutes hand

    }

}

