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

// TODO: Provare a scrivere più pin contemporaneamente manipolando i registri.


// Individuare il valore ideale per il delay tra uno step e l'altro (microsecondi)
const int staticDelay = 100;

// Individuare il valore ideale per il tempo di up dell'impulso per ogni step (microsecondi)
const int stepPulseMicrosec = 2;

SwitecX12::SwitecX12() {}

SwitecX12::SwitecX12(unsigned int steps, unsigned char pinStep[], unsigned char pinDir[], boolean reversed[])
{

    this->steps = steps;
    for (int i=0; i<MOTORS_COUNT; i++) {

        this->pinStep[i] = pinStep[i];
        this->pinDir[i] = pinDir[i];

        pinMode(pinStep[i], OUTPUT);
        pinMode(pinDir[i], OUTPUT);    

        digitalWrite(pinStep[i], LOW);
        digitalWrite(pinDir[i], LOW);

        reversedDirection[i] = reversed[i];
        stopped[i] = true;
        currentStep[i] = 0;
        targetStep[i] = 0;

    }

}

void SwitecX12::step()
{

    for (int i=0; i<MOTORS_COUNT; i++) {
        digitalWrite(pinDir[i], (dir[i] > 0) == reversedDirection[i] ? LOW : HIGH);
    }
    
    for (int i=0; i<MOTORS_COUNT; i++) {
        digitalWrite(pinStep[i], !stopped[i]);
    }
    
    delayMicroseconds(stepPulseMicrosec);
    
    for (int i=0; i<MOTORS_COUNT; i++) {
        digitalWrite(pinStep[i], LOW);
        currentStep[i] += dir[i];
    }

}

void SwitecX12::advance()
{

    for (int i=0; i<MOTORS_COUNT; i++) {
        if (currentStep[i]==targetStep[i]) {

            if (currentStep[i] >= steps) {
                currentStep[i] -= steps;
            }
            if (currentStep[i] < 0) {
                currentStep[i] += steps;
            }
            stopped[i] = true;
            dir[i] = 0;

        }
    }

    if (allStopped()) {
        time0 = 0;
        return;
    }

    step();

    time0 = micros();
}

void SwitecX12::setPosition(unsigned char motor, int step)
{

    while (step < 0) {
        step += steps;
    }
    while (step >= steps) {
        step -= steps;
    }

    targetStep[motor] = step;

    stopped[motor] = false;
    if (time0 == 0) {
        time0 = micros();
    }

    updateDirections();

}

void SwitecX12::updateDirections() {

    for (int i=0; i<MOTORS_COUNT; i++) {

        int delta = targetStep[i] - currentStep[i];
        int halfSteps = steps*0.5;

        if ( delta == 0 ) {
            dir[i] = 0;
        } else if ( delta < -halfSteps ) { // < -180

            // Modificare i targetStep andrebbe fatto in setPosition
            targetStep[i] += steps;
            dir[i] = 1;
        } else if (delta <= 0) {  // <= 0
            dir[i] = -1;
        } else if (delta <= halfSteps) {  // <= 180
            dir[i] = 1;
        } else { // > 180

            // Modificare i targetStep andrebbe fatto in setPosition
            targetStep[i] -= steps;
            dir[i] = -1;
        }

    }

}

void SwitecX12::setTargetRotation(unsigned char motor, float rot)
{
    // Aggiungere un parametro per forzare il verso orario/antiorario
    // Per ora prendo la più vicina
    setPosition(motor, rotationToSteps(rot));
}

float SwitecX12::getCurrentRotation(unsigned char motor) {
    return stepsToRotation(currentStep[motor]);
}

// void SwitecX12::setInitialRotation(float rot) {
//     currentStep = rotationToSteps(rot);
// }

int SwitecX12::rotationToSteps(float rot) {
    return (int)(rot / 360.0f * steps);
}

float SwitecX12::stepsToRotation(int pos) {
    return (float)pos * 360.0f / steps;
}

bool SwitecX12::allStopped() {
    bool allStopped = true;
    for (int i=0; i<MOTORS_COUNT; i++) {
        allStopped &= stopped[i];
    }
    return allStopped;
    //return stopped[0] && stopped[1] && stopped[2] && stopped[3];
}

void SwitecX12::update()
{
    if (!allStopped()) {
        unsigned long delta = micros() - time0;
        if (delta >= staticDelay) {
            advance();
        }
    }
}
