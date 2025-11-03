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

// TODO: Check pin nomenclature and/or assignment.
// I think they are inverted, if so the parameters of each coaxial must be swapped in pairs

/*

For each board I have in order the parameters of the hands:
- proximal left
- distal left
- proximal right
- distal right

*/

static unsigned short pins[48][3] = { // {DIR, STEP, REVERSED}
    // Board 0
    {  6, 13, 0},         // HOURS         TOPLEFT     HOURS_HAND
    {  7, 12, 1},         //                           MINUTES_HAND
    {  9, 10, 0},         //               TOPRIGHT    HOURS_HAND
    {  8, 11, 1},         //                           MINUTES_HAND
    // Board 1
    { 21,  5, 0},         //               MIDDLELEFT  HOURS_HAND
    { 20,  4, 1},         //                           MINUTES_HAND
    { 18,  2, 0},         //               MIDDLERIGHT HOURS_HAND
    { 19,  3, 1},         //                           MINUTES_HAND
    // Board 2
    { 29, 22, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 28, 23, 1},         //                           MINUTES_HAND
    { 26, 25, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 27, 24, 1},         //                           MINUTES_HAND

    // Board 3
    { 30, 37, 0},         // MINUTES       TOPLEFT     HOURS_HAND
    { 31, 36, 1},         //                           MINUTES_HAND
    { 33, 34, 0},         //               TOPRIGHT    HOURS_HAND
    { 32, 35, 1},         //                           MINUTES_HAND
    // Board 4
    { 38, 45, 0},         //               MIDDLELEFT  HOURS_HAND
    { 39, 44, 1},         //                           MINUTES_HAND
    { 41, 42, 0},         //               MIDDLERIGHT HOURS_HAND
    { 40, 43, 1},         //                           MINUTES_HAND
    // Board 5
    { 46, 50, 0},         //               BOTTOMLEFT  HOURS_HAND
    { 47, 51, 1},         //                           MINUTES_HAND
    { 49, 53, 0},         //               BOTTOMRIGHT HOURS_HAND
    { 48, 52, 1}          //                           MINUTES_HAND

};

