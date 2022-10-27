#include "SwitecX12.h"
#include "ClockPositions.h"
#include "ClockPins.h"

const int STEPS = 360 * 12;
const int RESET = 50;

/*

1  f(C)
2  DIR(C)
3  f(B)
4  DIR(B)
5  f(A)
6  DIR(A)
7  f(D)
8  DIR(D)
9  RESET
10 GROUND
11 +5v
12 unused

*/

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

    // Indagari qui, forse il problema è nella verifica che i motori siano allStopped

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

    for (int numberOffset = 0; numberOffset < 2; numberOffset++) {
        for (int i = 0; i < CONNECTED_BOARDS*0.5; i++) {
            boards[numberOffset * 3 + i].setTargetRotation(0, numbers[time[numberOffset] - '0'][i][0][0]);   // Left hour hand
            boards[numberOffset * 3 + i].setTargetRotation(1, numbers[time[numberOffset] - '0'][i][0][1]);   // Left minutes hand
            boards[numberOffset * 3 + i].setTargetRotation(2, numbers[time[numberOffset] - '0'][i][1][0]);   // Right hour hand
            boards[numberOffset * 3 + i].setTargetRotation(3, numbers[time[numberOffset] - '0'][i][1][1]);   // Right minutes hand
        }        
    }

}

