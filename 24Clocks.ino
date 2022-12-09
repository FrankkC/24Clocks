#include "SwitecX12.h"
#include "ClockPositions.h"
#include "ClockPins.h"

const int STEPS = 360 * 12;
const int RESET = 17;


#define CONNECTED_BOARDS 6

SwitecX12 boards[CONNECTED_BOARDS];
int timer = 0;

void setup() {

    Serial.begin(115200);

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        addBoard(i);
    }
    
    digitalWrite(RESET, HIGH);

    Serial.println("Set now hands to 0°");
    delay(5000); 
    
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


void loop() {

    bool allStopped = true;

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        boards[i].update();
        allStopped &= boards[i].allStopped();
    }

    if (allStopped) {

        delay(1000);
        if (timer == 9) {
            timer = 0;
            delay(4000);          
        } else {
            timer++;
        }
        char time[4];

        sprintf (time, "%d000", timer);
        setDisplayTime(time);
        
    }

}


void setDisplayTime(char* time) {

    Serial.print("setDisplayTime: ");
    Serial.println(time);

    for (int i = 0; i < CONNECTED_BOARDS; i++) {

        boards[i].setTargetRotation(0, numbers[time[0] - '0'][i%3][0][0]);   // Left hour hand
        boards[i].setTargetRotation(1, numbers[time[0] - '0'][i%3][0][1]);   // Left minutes hand
        boards[i].setTargetRotation(2, numbers[time[0] - '0'][i%3][1][0]);   // Right hour hand
        boards[i].setTargetRotation(3, numbers[time[0] - '0'][i%3][1][1]);   // Right minutes hand

    }

}

