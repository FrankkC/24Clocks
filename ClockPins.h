// TODO: Handle OFFSET for the second Arduino.

#define OFFSET_HOURS 0
#define OFFSET_MINUTES 24

#define OFFSET_TENS 0
#define OFFSET_UNITS 12

#define OFFSET_TOPLEFT 0
#define OFFSET_TOPRIGHT 2
#define OFFSET_MIDDLELEFT 4
#define OFFSET_MIDDLERIGHT 6
#define OFFSET_BOTTOMLEFT 8
#define OFFSET_BOTTOMRIGHT 10

#define OFFSET_HOURS_HAND 0
#define OFFSET_MINUTES_HAND 1


static unsigned short pins[48][3] = { // {DIR, STEP, REVERSED}
    // Board 0
    {  3,  7, 1},         // HOURS   TENS  TOPLEFT     HOURS_HAND
    {  2,  6, 0},         //                           MINUTES_HAND
    {  4,  8, 1},         //               TOPRIGHT    HOURS_HAND
    {  5,  9, 0},         //                           MINUTES_HAND
    // Board 1
    { 23, 11, 1},         //               MIDDLELEFT  HOURS_HAND
    { 22, 10, 0},         //                           MINUTES_HAND
    { 24, 12, 1},         //               MIDDLERIGHT HOURS_HAND
    { 25, 13, 0},         //                           MINUTES_HAND
    // Board 2
    { 26, 18, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 27, 19, 0},         //                           MINUTES_HAND
    { 28, 20, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 29, 21, 0},         //                           MINUTES_HAND
    // Board 3
    { 0, 0, 0},         //         UNITS TOPLEFT     HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               TOPRIGHT    HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    // Board 4
    { 0, 0, 0},         //               MIDDLELEFT  HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               MIDDLERIGHT HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    // Board 5
    { 0, 0, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    // Board 6
    { 34, 30, 0},         // MINUTES  TENS TOPLEFT     HOURS_HAND
    { 35, 31, 0},         //                           MINUTES_HAND
    { 36, 32, 0},         //               TOPRIGHT    HOURS_HAND
    { 37, 33, 0},         //                           MINUTES_HAND
    // Board 7
    { 42, 38, 0},         //               MIDDLELEFT  HOURS_HAND
    { 43, 39, 0},         //                           MINUTES_HAND
    { 44, 40, 0},         //               MIDDLERIGHT HOURS_HAND
    { 45, 41, 0},         //                           MINUTES_HAND
    // Board 8
    { 50, 46, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 51, 47, 0},         //                           MINUTES_HAND
    { 52, 48, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 53, 49, 0},         //                           MINUTES_HAND
    // Board 9
    { 0, 0, 0},         //         UNITS TOPLEFT     HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               TOPRIGHT    HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    // Board 10
    { 0, 0, 0},         //               MIDDLELEFT  HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               MIDDLERIGHT HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    // Board 11
    { 0, 0, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 0, 0, 0}          //                           MINUTES_HAND
};