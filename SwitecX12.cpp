/*
 *  SwitecX12 Arduino Library
 *  Guy Carpenter, Clearwater Software - 2017
 *
 *  Licensed under the BSD2 license, see license.txt for details.
 *
 *  All text above must be included in any redistribution.
 */

#include <Arduino.h>
#include "SwitecX12.h"

// This table defines the acceleration curve.
// 1st value is the speed step, 2nd value is delay in microseconds
// 1st value in each row must be > 1st value in subsequent row
// 1st value in last row should be == maxVel, must be <= maxVel
static unsigned short defaultAccelTable[][2] = {
    {   20, 800},
    {   50, 400},
    {  100, 200},
    {  150, 150},
    {  300, 90}
};

const int stepPulseMicrosec = 1;
const int resetStepMicrosec = 500;
#define DEFAULT_ACCEL_TABLE_SIZE (sizeof(defaultAccelTable)/sizeof(*defaultAccelTable))

SwitecX12::SwitecX12() {}

SwitecX12::SwitecX12(unsigned int steps, unsigned char pinStep, unsigned char pinDir)
{
    this->steps = steps;
    this->pinStep = pinStep;
    this->pinDir = pinDir;
    pinMode(pinStep, OUTPUT);
    pinMode(pinDir, OUTPUT);
    digitalWrite(pinStep, LOW);
    digitalWrite(pinDir, LOW);
    pinMode(13, OUTPUT);

    dir = 0;
    vel = 0;
    stopped = true;
    currentStep = 0;
    targetStep = 0;

    accelTable = defaultAccelTable;
    maxVel = defaultAccelTable[DEFAULT_ACCEL_TABLE_SIZE-1][0]; // last value in table.
}

void SwitecX12::step(int dir)
{
    digitalWrite(pinDir, (dir > 0) == reversedDirection ? LOW : HIGH);
    digitalWrite(13, vel == maxVel ? HIGH : LOW);
    digitalWrite(pinStep, HIGH);
    delayMicroseconds(stepPulseMicrosec);
    digitalWrite(pinStep, LOW);
    currentStep += dir;
}

void SwitecX12::stepTo(int position)
{
    int count;
    int dir;
    if (position > currentStep) {
        dir = 1;
        count = position - currentStep;
    } else {
        dir = -1;
        count = currentStep - position;
    }
    for (int i=0;i<count;i++) {
        step(dir);
        delayMicroseconds(resetStepMicrosec);
    }
}

void SwitecX12::zero()
{
    currentStep = steps - 1;
    stepTo(0);
    targetStep = 0;
    vel = 0;
    dir = 0;
}

void SwitecX12::advance()
{

    //Serial.println(stepsToRotation(currentStep));

    // detect stopped state
    if (currentStep==targetStep && vel==0) {

        // Serial.print("advance currentStep:");
        // Serial.println(currentStep);
        // Serial.println("-------------------------------------");

        if (currentStep >= steps) {
            currentStep -= steps;
        }
        if (currentStep < 0) {
            currentStep += steps;
        }
        stopped = true;
        dir = 0;
        time0 = micros();
        return;
    }

    // if stopped, determine direction
    if (vel==0) {
        updateDirection();
        // do not set to 0 or it could go negative in case 2 below
        vel = 1;
    }

    step(dir);

    // determine delta, number of steps in current direction to target.
    // may be negative if we are headed away from target
    int delta = dir>0 ? targetStep-currentStep : currentStep-targetStep;

    if (delta>0) {
        // case 1 : moving towards target (maybe under accel or decel)
        //Serial.println("case1");
        if (delta < vel) {
            // time to declerate
            vel = delta;
        } else if (vel < maxVel) {
            // accelerating
            vel++;
        } else {
            // at full speed - stay there
        }
    } else {
        // case 2 : at or moving away from target (slow down!)
        //Serial.println("case2");
        vel--;
    }

    // vel now defines delay
    unsigned char i = 0;
    // this is why vel must not be greater than the last vel in the table.
    while (accelTable[i][0]<vel) {
        i++;
    }
    microDelay = 4 * accelTable[i][1];
    time0 = micros();
}

void SwitecX12::setPosition(int pos)
{
    // Serial.print("setPosition pos:");
    // Serial.println(pos);
    // Serial.println("-------------------------------------");

    while (pos < 0) {
        pos += steps;
    }
    while (pos >= steps) {
        pos -= steps;
    }
    if (stopped) {
        // targetStep viene utilizzato in advance() e in updateDirection()
        targetStep = pos;
        // reset the timer to avoid possible time overflow giving spurious deltas
        stopped = false;
        time0 = micros();
        microDelay = 0;
    } else {
        // se sono in movimento dovrei gestire diversamente la richiesta.
        // Inoltre andrebbe probabilmente gestita diversamente se mi sto muovendo nel verso giusto o al contrario (prima mi fermo e poi riparto)
        // Per il momento non faccio nulla e fine.
    }

    // Serial.print("setPosition targetStep:");
    // Serial.println(targetStep);
    // Serial.println("-------------------------------------");

}

void SwitecX12::updateDirection() {

    // L'ideale sarebbe avere un parametro

    // Serial.print("updateDirection targetStep:");
    // Serial.println(targetStep);
    // Serial.println("-------------------------------------");

    // dir = 1;
    // return;

    int delta = targetStep - currentStep;
    int halfSteps = steps*0.5;

    // Serial.print("updateDirection delta:");
    // Serial.println(delta);
    // Serial.println("-------------------------------------");

    if ( delta < -halfSteps ) { // < -180
        targetStep += steps;
        dir = 1;
    } else if (delta <= 0) {  // <= 0
        dir = -1;
    } else if (delta <= halfSteps) {  // <= 180
        dir = 1;
    } else { // > 180
        targetStep -= steps;
        dir = -1;
    }

    // Serial.print(currentStep);
    // Serial.print(" --> ");
    // Serial.println(targetStep);
    // Serial.print("dir = ");
    // Serial.println(dir);
    // Serial.println("-------------------------------------");

}

void SwitecX12::setTargetRotation(float rot)
{
    // Aggiungere un parametro per forzare il verso orario/antiorario
    // Per ora prendo la più vicina
    setPosition(rotationToSteps(rot));
}

void SwitecX12::setInitialRotation(float rot) {
    currentStep = rotationToSteps(rot);
}

int SwitecX12::rotationToSteps(float rot) {
    return (int)(rot / 360.0f * steps);
}

float SwitecX12::stepsToRotation(int pos) {
    return (float)pos * 360.0f / steps;
}

void SwitecX12::setReversedDirection(bool reversed) {
    reversedDirection = reversed;
}

void SwitecX12::update()
{
    if (!stopped) {
        unsigned long delta = micros() - time0;
        if (delta >= microDelay) {
            advance();
        }
    }
}
