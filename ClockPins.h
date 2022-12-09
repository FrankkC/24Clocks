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
    { 20,  3, 1},         // HOURS   TENS  TOPLEFT     HOURS_HAND
    { 21,  2, 0},         //                           MINUTES_HAND
    { 19,  4, 1},         //               TOPRIGHT    HOURS_HAND
    { 18,  5, 0},         //                           MINUTES_HAND
    // Board 1
    { 24,  7, 1},         //               MIDDLELEFT  HOURS_HAND
    { 25,  6, 0},         //                           MINUTES_HAND
    { 23,  8, 1},         //               MIDDLERIGHT HOURS_HAND
    { 22,  9, 0},         //                           MINUTES_HAND
    // Board 2
    { 28, 11, 1},         //               BOTTOMLEFT  HOURS_HAND
    { 29, 10, 0},         //                           MINUTES_HAND
    { 27, 12, 1},         //               BOTTOMRIGHT HOURS_HAND
    { 26, 13, 0},         //                           MINUTES_HAND
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
    { 42, 38, 0},         // MINUTES  TENS TOPLEFT     HOURS_HAND
    { 43, 39, 0},         //                           MINUTES_HAND
    { 44, 40, 0},         //               TOPRIGHT    HOURS_HAND
    { 45, 41, 0},         //                           MINUTES_HAND
    // Board 7
    { 46, 34, 0},         //               MIDDLELEFT  HOURS_HAND
    { 47, 35, 0},         //                           MINUTES_HAND
    { 48, 36, 0},         //               MIDDLERIGHT HOURS_HAND
    { 49, 37, 0},         //                           MINUTES_HAND
    // Board 8
    { 50, 30, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 51, 31, 0},         //                           MINUTES_HAND
    { 52, 32, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 53, 33, 0},         //                           MINUTES_HAND
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