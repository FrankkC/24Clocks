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

#define CONNECTED_BOARDS 1

SwitecX12 boards[CONNECTED_BOARDS];
int timer = 0;

void setup() {

    Serial.begin(115200);

    addBoard(0);

    digitalWrite(RESET, HIGH);

    setDisplayTime("0000");

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


    int step1 = numbers[time[0] - '0'][0][0];
    int step2 = numbers[time[0] - '0'][0][1];
    int step3 = numbers[time[0] - '0'][1][0];
    int step4 = numbers[time[0] - '0'][1][1];

    //int step = (time[0] - '0') * 60;

// TODO: Usando lo stesso step per tutti i motori funziona regolarmente. Se al contrario metto valori diversi non funziona più.
// Pertanto l'idea è quella di modificare la libreria SwitecX12 per gestire simultaneamente i 4 motori collegati a un singolo X12 in sincrono. Con la speranza che questo ci consenta di controllarli regolarmente.
// Per verificare l'ipotesi dobbiamo collegare un secondo X12 ad Arduino. Se riesco a impostare direzioni diverse ai due X12 il problema è sicuramente collegato a come pilotiamo il singolo X12 per ottenere direzioni diverse per ognuno dei motori ad esso collegati

    boards[0].setTargetRotation(0, step1);
    boards[0].setTargetRotation(1, step2);
    boards[0].setTargetRotation(2, step3);
    boards[0].setTargetRotation(3, step4);
    // motors[offset2].setTargetRotation(step1);
    // motors[offset3].setTargetRotation(step1);
    // motors[offset4].setTargetRotation(step1);
    // motors[0].setTargetRotation(step);
    // motors[1].setTargetRotation(step);
    // motors[2].setTargetRotation(step);
    // motors[3].setTargetRotation(step);

}

