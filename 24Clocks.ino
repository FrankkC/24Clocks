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

    bool allStopped = false;

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        boards[i].update();
        allStopped |= boards[i].allStopped();
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

    // boards[0].setTargetRotation(0, numbers[time[0] - '0'][0][0][0]);
    // boards[0].setTargetRotation(1, numbers[time[0] - '0'][0][0][1]);
    // boards[0].setTargetRotation(2, numbers[time[0] - '0'][0][1][0]);
    // boards[0].setTargetRotation(3, numbers[time[0] - '0'][0][1][1]);

    // boards[1].setTargetRotation(0, numbers[time[0] - '0'][1][0][0]);
    // boards[1].setTargetRotation(1, numbers[time[0] - '0'][1][0][1]);
    // boards[1].setTargetRotation(2, numbers[time[0] - '0'][1][1][0]);
    // boards[1].setTargetRotation(3, numbers[time[0] - '0'][1][1][1]);

    // boards[2].setTargetRotation(0, numbers[time[0] - '0'][2][0][0]);
    // boards[2].setTargetRotation(1, numbers[time[0] - '0'][2][0][1]);
    // boards[2].setTargetRotation(2, numbers[time[0] - '0'][2][1][0]);
    // boards[2].setTargetRotation(3, numbers[time[0] - '0'][2][1][1]);

    // boards[3].setTargetRotation(0, numbers[time[1] - '0'][0][0][0]);
    // boards[3].setTargetRotation(1, numbers[time[1] - '0'][0][0][1]);
    // boards[3].setTargetRotation(2, numbers[time[1] - '0'][0][1][0]);
    // boards[3].setTargetRotation(3, numbers[time[1] - '0'][0][1][1]);

    // boards[4].setTargetRotation(0, numbers[time[1] - '0'][1][0][0]);
    // boards[4].setTargetRotation(1, numbers[time[1] - '0'][1][0][1]);
    // boards[4].setTargetRotation(2, numbers[time[1] - '0'][1][1][0]);
    // boards[4].setTargetRotation(3, numbers[time[1] - '0'][1][1][1]);

    // boards[5].setTargetRotation(0, numbers[time[1] - '0'][2][0][0]);
    // boards[5].setTargetRotation(1, numbers[time[1] - '0'][2][0][1]);
    // boards[5].setTargetRotation(2, numbers[time[1] - '0'][2][1][0]);
    // boards[5].setTargetRotation(3, numbers[time[1] - '0'][2][1][1]);

    // for (int numberOffset = 0; numberOffset < 2; numberOffset++) {
    //     for (int i = 0; i < CONNECTED_BOARDS*0.5; i++) {
    //         boards[numberOffset * 3 + i].setTargetRotation(0, numbers[time[numberOffset] - '0'][numberOffset * 3 + i][0]);   // Left hour hand
    //         boards[numberOffset * 3 + i].setTargetRotation(1, numbers[time[numberOffset] - '0'][numberOffset * 3 + i][1]);   // Left minutes hand
    //         boards[numberOffset * 3 + i].setTargetRotation(2, numbers[time[numberOffset] - '0'][numberOffset * 3 + i + 1][0]); // Right hour hand
    //         boards[numberOffset * 3 + i].setTargetRotation(3, numbers[time[numberOffset] - '0'][numberOffset * 3 + i + 1][1]); // Right minutes hand
    //     }        
    // }
    
    int step0000 = numbers[time[0] - '0'][0][0][0];
    int step0001 = numbers[time[0] - '0'][0][0][1];
    int step0010 = numbers[time[0] - '0'][0][1][0];
    int step0011 = numbers[time[0] - '0'][0][1][1];

    int step0100 = numbers[time[0] - '0'][1][0][0];
    int step0101 = numbers[time[0] - '0'][1][0][1];
    int step0110 = numbers[time[0] - '0'][1][1][0];
    int step0111 = numbers[time[0] - '0'][1][1][1];

    int step0200 = numbers[time[0] - '0'][2][0][0];
    int step0201 = numbers[time[0] - '0'][2][0][1];
    int step0210 = numbers[time[0] - '0'][2][1][0];
    int step0211 = numbers[time[0] - '0'][2][1][1];

    int step1000 = numbers[time[1] - '0'][0][0][0];
    int step1001 = numbers[time[1] - '0'][0][0][1];
    int step1010 = numbers[time[1] - '0'][0][1][0];
    int step1011 = numbers[time[1] - '0'][0][1][1];

    int step1100 = numbers[time[1] - '0'][1][0][0];
    int step1101 = numbers[time[1] - '0'][1][0][1];
    int step1110 = numbers[time[1] - '0'][1][1][0];
    int step1111 = numbers[time[1] - '0'][1][1][1];

    int step1200 = numbers[time[1] - '0'][2][0][0];
    int step1201 = numbers[time[1] - '0'][2][0][1];
    int step1210 = numbers[time[1] - '0'][2][1][0];
    int step1211 = numbers[time[1] - '0'][2][1][1];

    for (int i = 0; i < CONNECTED_BOARDS; i++) {
        boards[i].setTargetRotation(0, step0000);   // Left hour hand
        boards[i].setTargetRotation(1, step0001);   // Left minutes hand
        boards[i].setTargetRotation(2, step0010); // Right hour hand
        boards[i].setTargetRotation(3, step0011); // Right minutes hand
    }  
    

    // Uncommenting the following lines and shortening the previous loop by 1, breaks something...
    // boards[5].setTargetRotation(0, step1200);   // Left hour hand
    // boards[5].setTargetRotation(1, step1201);   // Left minutes hand
    // boards[5].setTargetRotation(2, step1210); // Right hour hand
    // boards[5].setTargetRotation(3, step1211); // Right minutes hand

}

