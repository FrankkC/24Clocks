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
#include "serialLink.h"

const int STEPS = 360 * 12;
const int RESETPIN = 17;


constexpr int CONNECTED_BOARDS = 6;

int slaveOffset = 0;

SwitecX12 boards[CONNECTED_BOARDS];
int timer = 0;
bool countMode = false;

void setup() {

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
    WifiManager::init();

}

void loop() {

    if (!countMode && millis() > 10000) {
        countMode = true;
    }

    bool allStopped = true;

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        boards[i].update();
        allStopped &= boards[i].allStopped();
    }

    if (allStopped && countMode && timer != (millis()/1000)%10) {
        timer = (millis()/1000)%10;
        char time[4];
        sprintf (time, "%d%d%d%d", timer, timer, timer, timer);
        setDisplayTime(time);
    }

    //delay(1000);
    handleWifiCommand();

}

void handleWifiCommand() {

    String command = WifiManager::readCommand();

    if (command != "") {

        if (command.startsWith("\r\n+IPD,")) {
            //SerialLink::sendLog("Got data from telnet: " + String(command));
        }

        if (command.indexOf("SETTIME=") != -1) {
            String newTime = command.substring(command.indexOf("TIME=") + 5, command.indexOf("TIME=") + 9);
            setDisplayTime(newTime.c_str());
            WifiManager::sendData("SET TIME OK\r\n");
        } else if (command.indexOf("SETHOME") != -1) {
            setHome();
            WifiManager::sendData("SET HOME OK\r\n");
        } else if (command.indexOf("SETZERO") != -1) {
            setDisplayTime("0000");
            WifiManager::sendData("SET ZERO OK\r\n");
        } else if (command.indexOf("SETCOUNT=1") != -1) {
            countMode = true;
            WifiManager::sendData("SET COUNT MODE ON OK\r\n");
        } else if (command.indexOf("SETCOUNT=0") != -1) {
            countMode = false;
            WifiManager::sendData("SET COUNT MODE OFF OK\r\n");
        } else if (command.indexOf("ECHO") != -1) {
            WifiManager::sendData("ECHO OK\r\n");
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

void setLocalDisplayTime(const char* time) {
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


void setDisplayTime(const char* time) {
    SerialLink::sendLog("setDisplayTime: " + String(time));
    SerialLink::sendCommand("SETTIME=" + String(time));
    setLocalDisplayTime(time);
}

void setHome() {
    SerialLink::sendLog("Going home...");
    SerialLink::sendCommand("SETHOME");
    setLocalHome();
}
