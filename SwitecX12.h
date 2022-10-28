#ifndef SwitecX12_h
#define SwitecX12_h
#include <Arduino.h>

class SwitecX12 {
    public:

        unsigned int steps;      // total steps available
        unsigned char pinStep[4];
        unsigned char pinDir[4];
        int currentStep[4];      // step we are currently at
        int targetStep[4];       // target we are moving to
        unsigned long time0 = 0; // time when we entered this state
        int dir[4];              // direction -1,0,1
        boolean stopped[4];
        boolean reversedDirection[4];
        
        SwitecX12();
        SwitecX12(unsigned int steps, unsigned char pinStep[], unsigned char pinDir[], boolean reversed[]);

        void updateDirections();
        void step();
        // void zero();
        // void stepTo(int position);
        void advance();
        void update();
        bool allStopped();
        void setPosition(unsigned char motor, int pos);
        void setTargetRotation(unsigned char motor, float rot);
        float getCurrentRotation(unsigned char motor);
        // void setInitialRotation(float rot);
        int rotationToSteps(float rot);
        float stepsToRotation(int pos);
        // void setReversedDirection(bool reversed);
};

#endif
