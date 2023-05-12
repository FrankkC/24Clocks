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
#include "wifiManager.h"

const int STEPS = 360 * 12;
const int RESETPIN = 17;


constexpr int CONNECTED_BOARDS = 6;
#define SLAVE
int slaveOffset = 1;

#define SLAVE_MONITOR Serial
#define SM_SERIAL Serial3

SwitecX12 boards[CONNECTED_BOARDS];
int timer = 0;
bool countMode = false;

void setup() {

    SM_SERIAL.begin(115200);
    SLAVE_MONITOR.begin(115200);


    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        addBoard(i);
    }
    
    digitalWrite(RESETPIN, HIGH);

}

void loop() {

    bool allStopped = true;

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        boards[i].update();
        allStopped &= boards[i].allStopped();
    }

    if (allStopped && countMode) {

        delay(500);
        if (timer == 9) {
            timer = 0;
            delay(1000);
        } else {
            timer++;
        }
        char time[4];

        sprintf (time, "%d0%d0", timer, timer);
        setDisplayTime(time);
        
    }

    String incomingString = "";
    bool stringReady = false;

    while (Serial2.available()) {
        incomingString = Serial2.readString();
        stringReady = true;
    }

    if (stringReady) {

        if (incomingString.indexOf("AAAAA") != -1) {

        }

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

void setDisplayTime(const char* time) {

    //Serial.print("setDisplayTime: ");
    //Serial.println(time);

    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        int timeDigit = (i/3) * 2 + slaveOffset;

        boards[i].setTargetRotation(0, numbers[time[timeDigit] - '0'][i%3][0][0]);   // Left hour hand
        boards[i].setTargetRotation(1, numbers[time[timeDigit] - '0'][i%3][0][1]);   // Left minutes hand
        boards[i].setTargetRotation(2, numbers[time[timeDigit] - '0'][i%3][1][0]);   // Right hour hand
        boards[i].setTargetRotation(3, numbers[time[timeDigit] - '0'][i%3][1][1]);   // Right minutes hand

    }

}

