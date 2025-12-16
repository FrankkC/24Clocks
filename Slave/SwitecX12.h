#ifndef SwitecX12_h
#define SwitecX12_h
#include <Arduino.h>

constexpr uint8_t MOTORS_COUNT = 4;

class SwitecX12 {
    public:

        int steps;                          // total steps available
        unsigned char pinStep[MOTORS_COUNT];
        unsigned char pinDir[MOTORS_COUNT];
        int currentStep[MOTORS_COUNT];      // step we are currently at
        int targetStep[MOTORS_COUNT];       // target we are moving to
        unsigned long time0 = 0;            // time when we entered this state
        int dir[MOTORS_COUNT];              // direction -1,0,1
        boolean stopped[MOTORS_COUNT];
        boolean reversedDirection[MOTORS_COUNT];
        
        SwitecX12();
        SwitecX12(unsigned int steps, unsigned char pinStep[], unsigned char pinDir[], boolean reversed[]);

        void updateDirections();
        void step();
        void advance();
        void update();
        bool allStopped();
        void setPosition(unsigned char motor, int pos);
        void setTargetRotation(unsigned char motor, float rot);
        float getCurrentRotation(unsigned char motor);
        void fineTune(unsigned char motor, float rot);
        void manualStep(unsigned char motor, int dir);
        // void setInitialRotation(float rot);
        int rotationToSteps(float rot);
        float stepsToRotation(int pos);
};

#endif
