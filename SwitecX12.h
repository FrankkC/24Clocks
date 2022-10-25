#ifndef SwitecX12_h
#define SwitecX12_h
#include <Arduino.h>

class SwitecX12 {
    public:
        unsigned char pinStep;
        unsigned char pinDir;
        int currentStep;      // step we are currently at
        int targetStep;       // target we are moving to
        int travelSteps = 0;
        unsigned int steps;            // total steps available
        unsigned long time0;           // time when we entered this state
        unsigned int microDelay;       // microsecs until next state
        unsigned short (*accelTable)[2]; // accel table can be modified.
        unsigned int maxVel;           // fastest vel allowed
        unsigned int vel;              // steps travelled under acceleration
        int dir;                      // direction -1,0,1
        boolean stopped;               // true if stopped
        boolean reversedDirection = false;
        
        SwitecX12();
        SwitecX12(unsigned int steps, unsigned char pinStep, unsigned char pinDir);

        //void stepUp();
        void updateDirection();
        void step(int dir);
        void zero();
        void stepTo(int position);
        void advance();
        void update();
        void setPosition(int pos);
        void setTargetRotation(float rot);
        void setInitialRotation(float rot);
        int rotationToSteps(float rot);
        float stepsToRotation(int pos);
        void setReversedDirection(bool reversed);
};

#endif
