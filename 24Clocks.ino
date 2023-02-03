#include <Arduino.h>
#include "SwitecX12.h"
#include "ClockPositions.h"
#include "ClockPins.h"
#include "wifiManager.h"

const int STEPS = 360 * 12;
const int RESET = 17;


constexpr int CONNECTED_BOARDS = 6;
#define MASTER

SwitecX12 boards[CONNECTED_BOARDS];
int timer = 0;
bool countMode = false;

void setup() {

    Serial.begin(115200);

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        addBoard(i);
    }
    
    digitalWrite(RESET, HIGH);

    //Serial.println("Set now hands to 0°");
    //delay(5000);
#ifdef MASTER
    WifiManager::init();
#endif

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

#ifdef MASTER
    String incomingString = "";
    bool stringReady = false;

    while (Serial3.available()) {
        incomingString = Serial3.readString();
        stringReady = true;
    }

    if (stringReady) {
        if (incomingString.startsWith("\r\n+IPD,")) {
            Serial.println("Got data from telnet: " + incomingString);
        }

        if (incomingString.indexOf("SETTIME=") != -1) {
            String newTime = incomingString.substring(incomingString.indexOf("TIME=") + 5, incomingString.indexOf("TIME=") + 9);
            Serial.println("Time to set: " + newTime);
            setDisplayTime(newTime.c_str());
            WifiManager::sendData("SET TIME OK\r\n");
        } else if (incomingString.indexOf("SETHOME") != -1) {
            
            Serial.println("Going home...");

            for (int i = 0; i < CONNECTED_BOARDS; i++) {

                boards[i].setTargetRotation(0, 0);   // Left hour hand
                boards[i].setTargetRotation(1, 0);   // Left minutes hand
                boards[i].setTargetRotation(2, 0);   // Right hour hand
                boards[i].setTargetRotation(3, 0);   // Right minutes hand

            }

            WifiManager::sendData("SET HOME OK\r\n");
        } else if (incomingString.indexOf("SETZERO") != -1) {
            WifiManager::sendData("SET ZERO OK (not implemented)\r\n");
        } else if (incomingString.indexOf("SETCOUNT=1") != -1) {
            countMode = 1;
            WifiManager::sendData("SET COUNT MODE ON OK\r\n");
        } else if (incomingString.indexOf("SETCOUNT=0") != -1) {
            countMode = 0;
            WifiManager::sendData("SET COUNT MODE OFF OK\r\n");
        } else if (incomingString.indexOf("ECHO") != -1) {
            WifiManager::sendData("ECHO OK\r\n");
        }

    }
#endif

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

    Serial.print("setDisplayTime: ");
    Serial.println(time);

    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        int timeDigit = (i/3) * 2;
        #ifndef MASTER
            timeDigit++;
        #endif

        boards[i].setTargetRotation(0, numbers[time[timeDigit] - '0'][i%3][0][0]);   // Left hour hand
        boards[i].setTargetRotation(1, numbers[time[timeDigit] - '0'][i%3][0][1]);   // Left minutes hand
        boards[i].setTargetRotation(2, numbers[time[timeDigit] - '0'][i%3][1][0]);   // Right hour hand
        boards[i].setTargetRotation(3, numbers[time[timeDigit] - '0'][i%3][1][1]);   // Right minutes hand

    }

}

