#include <Arduino.h>
#include "SwitecX12.h"
#include "ClockPositions.h"
#include "ClockPins.h"
#include "wifiManager.h"

const int STEPS = 360 * 12;
const int RESETPIN = 17;


constexpr int CONNECTED_BOARDS = 6;
#define MASTER
//#define SLAVE

#ifdef MASTER
#define USE_WIFI
#define MASTER_MONITOR Serial
#define MS_SERIAL Serial
#endif

#ifdef SLAVE
#define SLAVE_MONITOR Serial
#define SM_SERIAL Serial3
#endif

SwitecX12 boards[CONNECTED_BOARDS];
int timer = 0;
bool countMode = false;

void setup() {

    // Used in Master to connect to Slave
    // Used in Slave for the serial monitor
#ifdef MASTER
    MS_SERIAL.begin(115200);
#endif
#ifdef SLAVE
    SM_SERIAL.begin(115200);
    SLAVE_MONITOR.begin(115200);
#endif

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        addBoard(i);
    }
    
    digitalWrite(RESETPIN, HIGH);

    //Serial.println("Set now hands to 0°");
    //delay(5000);
#ifdef USE_WIFI
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

#ifdef USE_WIFI

    String command = "";

    if (command = WifiManager::readCommand() != "") {

        if (command.startsWith("\r\n+IPD,")) {
            //Serial.println("Got data from telnet: " + command);
        }

        if (command.indexOf("SETTIME=") != -1) {
            String newTime = command.substring(command.indexOf("TIME=") + 5, command.indexOf("TIME=") + 9);
            //Serial.println("Time to set: " + newTime);
            setDisplayTime(newTime.c_str());
            WifiManager::sendData("SET TIME OK\r\n");
        } else if (command.indexOf("SETHOME") != -1) {
            
            //Serial.println("Going home...");

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

#endif

#ifdef SLAVE
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

    //Serial.print("setDisplayTime: ");
    //Serial.println(time);

    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        int timeDigit = (i/3) * 2;
        #ifdef SLAVE
            timeDigit++;
        #endif

        boards[i].setTargetRotation(0, numbers[time[timeDigit] - '0'][i%3][0][0]);   // Left hour hand
        boards[i].setTargetRotation(1, numbers[time[timeDigit] - '0'][i%3][0][1]);   // Left minutes hand
        boards[i].setTargetRotation(2, numbers[time[timeDigit] - '0'][i%3][1][0]);   // Right hour hand
        boards[i].setTargetRotation(3, numbers[time[timeDigit] - '0'][i%3][1][1]);   // Right minutes hand

    }

}

