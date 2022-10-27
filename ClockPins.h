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
    { 10, 14, 0},         //               MIDDLELEFT  HOURS_HAND
    { 11, 15, 0},         //                           MINUTES_HAND
    { 12, 16, 0},         //               MIDDLERIGHT HOURS_HAND
    { 13, 17, 0},         //                           MINUTES_HAND
    // Board 2
    { 18, 22, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 19, 23, 0},         //                           MINUTES_HAND
    { 20, 24, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 21, 25, 0},         //                           MINUTES_HAND
    // Board 3
    { 26, 30, 0},         //         UNITS TOPLEFT     HOURS_HAND
    { 27, 31, 0},         //                           MINUTES_HAND
    { 28, 32, 0},         //               TOPRIGHT    HOURS_HAND
    { 29, 33, 0},         //                           MINUTES_HAND
    // Board 4
    { 34, 38, 0},         //               MIDDLELEFT  HOURS_HAND
    { 35, 39, 0},         //                           MINUTES_HAND
    { 36, 40, 0},         //               MIDDLERIGHT HOURS_HAND
    { 37, 41, 0},         //                           MINUTES_HAND
    // Board 5
    { 42, 46, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 43, 47, 0},         //                           MINUTES_HAND
    { 44, 48, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 45, 49, 0},         //                           MINUTES_HAND
    // Board 6
    { 0, 0, 0},         // MINUTES  TENS TOPLEFT     HOURS_HAND
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