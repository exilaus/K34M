
/*

  ============================================================================================
     NAMEBOARD
  ============================================================================================
#ifdef BOARD_NAMEBOARD

// motors PIN
#define xenable 2
#define xdirection 6
#define xstep 4

#define yenable 7
#define ydirection 9
#define ystep 8

#define zenable 10
#define zdirection A5
#define zstep A4

#define e0enable 11
#define e0direction A2
#define e0step A3


// ENDSTOPS PIN, can be using just 1 pin
#define limit_pin 0

#define temp_pin 6
#define temp2_pin 6
#define heater_pin 3
#define heater2_pin 3

#define DRIVE_XYYZ
#define DRIVE_COREXY
#define DRIVE_COREXZ
#define DRIVE_DELTA
#define DRIVE_DELTASIAN

#define ISRTEMP    // avr reading using interrupt
#define USETIMER1  // using timer1 or other timer (implemented timer1 on avr)
#define CORESERIAL // reduce code on AVR
#define OLEDDISPLAY // still WIP
#define SDCARD_CS // pin for SDCARD
#define KBOX_PIN // 4 key Kontrolbox using analog pin and serial resistors

#define MOTOR_0_DIR 1 // 1: normal -1:inverted
#define MOTOR_1_DIR 1 // 1: normal -1:inverted
#define MOTOR_2_DIR 1 // 1: normal -1:inverted
#define MOTOR_3_DIR 1 // 1: normal -1:inverted

#define USE_EEPROM
*/


/*

  ============================================================================================
     TARANTHOLE
  ============================================================================================
*/
#ifdef BOARD_TARANTHOLE

#define xenable 2
#define xdirection 6
#define xstep 4

#define yenable 7
#define ydirection 9
#define ystep 8

#define zenable 10
#define zdirection A5
#define zstep A4

#define e0enable 11
#define e0direction A2
#define e0step A3



#define limit_pin 13

#define temp_pin 6
#define heater_pin 3


/*
  ============================================================================================
////////////////////////////////////////////////////////////
////////////K34M Board
///////////////////////////////////////////////////////////////   
  ============================================================================================
*/
#elif defined(BOARD_WEMOS3D_Plasma)


// 
// Added static const uint8_t definition for D0-D10
//
//static const uint8_t D0   = 16;
//static const uint8_t D1   = 5;
//static const uint8_t D2   = 4;
//static const uint8_t D3   = 0;
//static const uint8_t D4   = 2;
//static const uint8_t D5   = 14;
//static const uint8_t D6   = 12;
//static const uint8_t D7   = 13;
//static const uint8_t D8   = 15;
//static const uint8_t D9   = 3;h
//static const uint8_t D10  = 1;
//


#define xdirection 14 
#define xstep  0 
#define ydirection 12 
#define ystep 2 

// z and e have same direction pin, we think that normally slicer never move z and e together.. we hope we are right :D
#define zdirection 13
#define zstep 16
#define e0direction 13
#define e0step 4 

#define limit_pin 15 

#define temp_pin A0
#define heater_pin 5
//#define laser_pin D1
//#define fan_pin D1 // To be used for plama relay/contact

//#define INVERTENDSTOP
#define NUMBUFFER 60 // increased buffer from 20 to 80

////////////////////////////////////////////////////////////
////////////End  K34M Board
///////////////////////////////////////////////////////////////

#else
#error No BOARD Defined !
#endif
