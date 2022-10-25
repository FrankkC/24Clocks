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

const int staticDelay = 4 * 0;

const int stepPulseMicrosec = 1;
const int resetStepMicrosec = 500;

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

    dir = 0;
    stopped = true;
    currentStep = 0;
    targetStep = 0;
}

void SwitecX12::step(int dir)
{
    digitalWrite(pinDir, (dir > 0) == reversedDirection ? LOW : HIGH);
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
    dir = 0;
}

void SwitecX12::advance()
{

    if (currentStep==targetStep) {

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

    updateDirection();
    step(dir);

    time0 = micros();
}

void SwitecX12::setPosition(int step)
{

    while (step < 0) {
        step += steps;
    }
    while (step >= steps) {
        step -= steps;
    }

    targetStep = step;

    if (stopped) {
        stopped = false;
        time0 = micros();
    }

}

void SwitecX12::updateDirection() {

    // L'ideale sarebbe avere un parametro

    int delta = targetStep - currentStep;
    int halfSteps = steps*0.5;

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
        if (delta >= staticDelay) {
            advance();
        }
    }
}
