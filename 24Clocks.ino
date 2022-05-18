#include "SwitecX12.h"

const int STEPS = 360 * 12;

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

const int RESET = 2;
const int D_DIR = 3;
const int D_STEP = 4;
const int A_DIR = 5;
const int A_STEP = 6;
const int B_DIR = 7;
const int B_STEP = 8;
const int C_DIR = 9;
const int C_STEP = 10;
const int COMMON_DIR = 11;
const int NULL_DIR = 12;

SwitecX12 motor1(STEPS, A_STEP, A_DIR);
SwitecX12 motor2(STEPS, B_STEP, B_DIR);
SwitecX12 motor3(STEPS, C_STEP, C_DIR);
SwitecX12 motor4(STEPS, D_STEP, D_DIR);


void setup() {

  digitalWrite(RESET, HIGH);
  Serial.begin(9600);

  //motor1.setInitialRotation(0.0);
  motor1.setTargetRotation(90);

  //motor2.setInitialRotation(0.0);
  motor2.setTargetRotation(90);

  //motor3.setInitialRotation(0.0);
  motor3.setTargetRotation(90);

  //motor4.setInitialRotation(0.0);
  motor4.setTargetRotation(90);

  
}

void loop() {

  /*static bool forward = true;
  static int position1 = STEPS;
  static int position2 = 0;
  if (motor1.stopped && motor2.stopped && motor3.stopped && motor4.stopped) {
    motor1.setPosition(forward ? position1 : position2);
    forward = !forward;
  }*/
  //if (motor2.stopped) {
    
    // float targetRotation = 10 * random(0,36);
    // Serial.print("Next target rotation: ");
    // Serial.print(targetRotation);
    // Serial.print("\n\n");


    //motor2.setTargetRotation(90);

    //delay(2000);
  //}

  motor1.update();
  motor2.update();
  motor3.update();
  motor4.update();

}
