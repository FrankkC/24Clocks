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
    {  6, 13, 1},         // HOURS   TENS  TOPLEFT     HOURS_HAND
    {  7, 12, 0},         //                           MINUTES_HAND
    {  8, 11, 1},         //               TOPRIGHT    HOURS_HAND
    {  9, 10, 0},         //                           MINUTES_HAND
    // Board 1
    { 21,  5, 1},         //               MIDDLELEFT  HOURS_HAND
    { 20,  4, 0},         //                           MINUTES_HAND
    { 19,  3, 1},         //               MIDDLERIGHT HOURS_HAND
    { 18,  2, 0},         //                           MINUTES_HAND
    // Board 2
    { 29, 22, 1},         //               BOTTOMLEFT  HOURS_HAND
    { 28, 23, 0},         //                           MINUTES_HAND
    { 27, 24, 1},         //               BOTTOMRIGHT HOURS_HAND
    { 26, 25, 0},         //                           MINUTES_HAND

    // Board 3
    { 30, 37, 1},         // MINUTES  TENS TOPLEFT     HOURS_HAND
    { 31, 36, 0},         //                           MINUTES_HAND
    { 32, 35, 1},         //               TOPRIGHT    HOURS_HAND
    { 33, 34, 0},         //                           MINUTES_HAND
    // Board 4
    { 38, 45, 1},         //               MIDDLELEFT  HOURS_HAND
    { 39, 44, 0},         //                           MINUTES_HAND
    { 40, 43, 1},         //               MIDDLERIGHT HOURS_HAND
    { 41, 42, 0},         //                           MINUTES_HAND
    // Board 5
    { 46, 50, 1},         //               BOTTOMLEFT  HOURS_HAND
    { 47, 51, 0},         //                           MINUTES_HAND
    { 48, 52, 1},         //               BOTTOMRIGHT HOURS_HAND
    { 49, 53, 0},         //                           MINUTES_HAND

    // Board 6
    { 0, 0, 0},         //         UNITS TOPLEFT     HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               TOPRIGHT    HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    // Board 7
    { 0, 0, 0},         //               MIDDLELEFT  HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               MIDDLERIGHT HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    // Board 8
    { 0, 0, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND
    { 0, 0, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 0, 0, 0},         //                           MINUTES_HAND

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