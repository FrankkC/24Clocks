#include "SwitecX12.h"
#include "ClockPositions.h"
#include "ClockPins.h"

const int STEPS = 360 * 12;
const int RESET = 2;

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

#define CONNECTED_MOTORS 4

SwitecX12 motors[CONNECTED_MOTORS];
int timer = 0;

void setup() {

    digitalWrite(RESET, HIGH);

    Serial.begin(115200);

    addMotor(OFFSET_HOURS, OFFSET_TENS, OFFSET_TOPLEFT, OFFSET_HOURS_HAND);
    addMotor(OFFSET_HOURS, OFFSET_TENS, OFFSET_TOPLEFT, OFFSET_MINUTES_HAND);
    addMotor(OFFSET_HOURS, OFFSET_TENS, OFFSET_TOPRIGHT, OFFSET_HOURS_HAND);
    addMotor(OFFSET_HOURS, OFFSET_TENS, OFFSET_TOPRIGHT, OFFSET_MINUTES_HAND);



    setDisplayTime("0000");
    
}

void loop() {

    bool allStopped = false;

    for (int i = 0; i < CONNECTED_MOTORS; i++) {
        motors[i].update();
        allStopped |= motors[i].stopped;
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

void addMotor(char offsetHours, char offsetTens, char offsetTopleft, char offsetHoursHand) {

    char totalOffset = offsetHours + offsetTens + offsetTopleft + offsetHoursHand;
    motors[totalOffset] = SwitecX12(STEPS, pins[totalOffset][1], pins[totalOffset][0]);

    if ( pins[totalOffset][2] ) {
        motors[totalOffset].setReversedDirection(true);
    }

    motors[totalOffset].setInitialRotation(0);

}

void setDisplayTime(char* time) {

    // Spostamenti in avanti (entro che range?) funzionano regolarmente.
    // Da sistemare gli spostamenti in direzione negativa
    // Al momento qualsiasi cosa >= 360 viene riportata nel range 0-359

    Serial.print("setDisplayTime");
    Serial.println(time);

    int step = (time[0] - '0') * 60;

    Serial.print("advance currentStep:");
    Serial.println(step);

    // motors[OFFSET_HOURS + OFFSET_TENS + OFFSET_TOPLEFT + OFFSET_HOURS_HAND].setTargetRotation(numbers[time[0] - '0'][0][0]);
    // motors[OFFSET_HOURS + OFFSET_TENS + OFFSET_TOPLEFT + OFFSET_MINUTES_HAND].setTargetRotation(numbers[time[0] - '0'][0][1]);
    // motors[OFFSET_HOURS + OFFSET_TENS + OFFSET_TOPRIGHT + OFFSET_HOURS_HAND].setTargetRotation(numbers[time[0] - '0'][1][0]);
    // motors[OFFSET_HOURS + OFFSET_TENS + OFFSET_TOPRIGHT + OFFSET_MINUTES_HAND].setTargetRotation(numbers[time[0] - '0'][1][1]);

    motors[0].setTargetRotation(step);
    motors[1].setTargetRotation(step);
    motors[2].setTargetRotation(step);
    motors[3].setTargetRotation(step);

}

