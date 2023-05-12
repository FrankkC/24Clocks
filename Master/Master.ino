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

int slaveOffset = 0;

SwitecX12 boards[CONNECTED_BOARDS];
int timer = 0;
bool countMode = false;

void setup() {

    MASTER_SLAVE_SERIAL.begin(115200);

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        addBoard(i);
    }
    
    digitalWrite(RESETPIN, HIGH);

    WifiManager::init();

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

    delay(1000);
    handleWifiCommand();

}

void handleWifiCommand() {

    String command = WifiManager::readCommand();

    if (command != "") {

        if (command.startsWith("\r\n+IPD,")) {
            //serialMonitor("Got data from telnet: " + String(command));
        }

        if (command.indexOf("SETTIME=") != -1) {
            String newTime = command.substring(command.indexOf("TIME=") + 5, command.indexOf("TIME=") + 9);
            //serialMonitor("Time to set: " + String(newTime));
            setDisplayTime(newTime.c_str());
            WifiManager::sendData("SET TIME OK\r\n");
        } else if (command.indexOf("SETHOME") != -1) {
            
            //serialMonitor("Going home...");

            for (int i = 0; i < CONNECTED_BOARDS; i++) {

                boards[i].setTargetRotation(0, 0);   // Left hour hand
                boards[i].setTargetRotation(1, 0);   // Left minutes hand
                boards[i].setTargetRotation(2, 0);   // Right hour hand
                boards[i].setTargetRotation(3, 0);   // Right minutes hand

            }

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
            countMode = true;
            WifiManager::sendData("ECHO OK\r\n");
        }

    }

}

void serialMonitor(const String &s) {
    MASTER_SLAVE_SERIAL.println(s+'\n');
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

