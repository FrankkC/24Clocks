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
  { 3, 4, 0},         // HOURS   TENS  TOPLEFT     HOURS_HAND
  { 5, 6, 1},         //                           MINUTES_HAND
  { 7, 8, 1},         //               TOPRIGHT    HOURS_HAND
  { 9, 10, 0},        //                           MINUTES_HAND
  { 0, 0, 0},         //               MIDDLELEFT  HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               MIDDLERIGHT HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               BOTTOMLEFT  HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               BOTTOMRIGHT HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //         UNITS TOPLEFT     HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               TOPRIGHT    HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               MIDDLELEFT  HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               MIDDLERIGHT HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               BOTTOMLEFT  HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               BOTTOMRIGHT HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         // MINUTES UNITS TOPLEFT     HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               TOPRIGHT    HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               MIDDLELEFT  HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               MIDDLERIGHT HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               BOTTOMLEFT  HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               BOTTOMRIGHT HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //         UNITS TOPLEFT     HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               TOPRIGHT    HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               MIDDLELEFT  HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               MIDDLERIGHT HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               BOTTOMLEFT  HOURS_HAND
  { 0, 0, 0},         //                           MINUTES_HAND
  { 0, 0, 0},         //               BOTTOMRIGHT HOURS_HAND
  { 0, 0, 0}          //                           MINUTES_HAND
};