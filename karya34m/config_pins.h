//wifi_telebot="";
//wifi_ap="boardk34M";
//wifi_pwd="K34M";
//wifi_dns="K34M";

/*
  ============================================================================================
    AVR
  ============================================================================================
*/

#include "motion.h"
#ifndef ISPC
#include<arduino.h>

//Known board in boards.h
#define xenable -1
#define yenable -1
#define zenable -1
#define e0enable -1

#define ISRTEMP // 120bytes check board.h
#define MOTOR_0_DIR -1 // 1: normal -1:inverted
#define MOTOR_1_DIR -1 // 1: normal -1:inverted
#define MOTOR_2_DIR -1 // 1: normal -1:inverted
#define MOTOR_3_DIR 1 // 1: normal -1:inverted
#define THEISR

// ====== ESP8266 ====================================================
#if defined(ESP8266)
//#define SUBPIXELMAX 6  // multiple axis smoothing / AMASS maximum subpixel
#define THEISR ICACHE_RAM_ATTR 
#define ANALOGSHIFT 0 // 10bit adc ??



#define BOARD_K34M_Plasma
//#define BOARD_K34M_
#endif

#include "boards.h"
#define USE_EEPROM

#else
// for PC no pins
#endif


/*
  ============================================================================================
    CONFIGURATION
  ============================================================================================
*/

//#define ARC_SUPPORT // 3kb

#define USEDIO // 750bytes this can save almost 20us each bresenham step, is a MUST if not using timer!
#define USE_BACKLASH  // 400bytes code
#define USETIMER1 // Work in progress // 98 bytes// FLASH SAVING
//#define SAVE_RESETMOTION  // 1000 bytes code, no reset motion, need EEPROM
//#define LCDDISPLAY 0x3F // more than 2.5K , simple oled controller
#define CORESERIAL // smaller footprint 500byte, only AVR
#define CHANGEFILAMENT //580byte
#define HARDSTOP // allow to stop in the middle of movement, and still keep the current position, great for CNC
#define WIFISERVER
//#define TELEGRAM
// ==========================================================

#define INTERPOLATEDELAY  // slower 4-8us

#ifdef powerpin
#define POWERFAILURE
#endif

// lets assume if not laser_pin not defined use the heater_pin 

#ifdef laser_pin
#define LASERMODE
#elif defined(heater_pin)

// to make sure all board can be user for laser engraving
#define laser_pin heater_pin
#define LASERMODE
#endif


#define UPDATE_F_EVERY 2000 //us = 250 tick/sec acceleration change
#ifndef ISPC
//#define SUBPIXELMAX 6  // multiple axis smoothing / AMASS maximum subpixel
#else
//#define SUBPIXELMAX 4
#endif





// ESP8266
#if defined(ESP8266) && !defined(USE_SHIFTREG)
#define SHARE_EZ
#endif


//#ifndef ESP8266
//
//
//Disabled webserver for testing purposes
//
//#ifdef ESP8266
//#undef WIFISERVER


#ifndef __AVR__

// not implemented on non AVR
#undef USEDIO
#undef ISRTEMP
#undef CORESERIAL
//#undef USETIMER1

#endif

#ifdef ISPC
// not implemented on PC
#undef USETIMER1
#undef LASERMODE
#undef SAVE_RESETMOTION
#endif


//#define motortimeout 10000000 // 10 seconds

//#define DRIVE_XYYZ  // dual Y individual homing
//#define DRIVE_COREXY
//#define DRIVE_COREXZ

//#define DRIVE_DELTA
//#define DRIVE_DELTASIAN


#ifdef DRIVE_DELTA
#define NONLINEAR
#endif
#ifdef DRIVE_DELTASIAN
#define NONLINEAR
#endif


#define TOWER_X_ANGLE_DEG        210
#define TOWER_Y_ANGLE_DEG        330
#define TOWER_Z_ANGLE_DEG        90
#define DELTA_DIAGONAL_ROD 180
#define DELTA_RADIUS 85

// Motion configuration
#define CHECKENDSTOP_EVERY 0.05  // mm this translate to 200step if step/mm is 4000, must lower than 255 (byte size)
#define HOMINGSPEED 60
#define XOFFSET 0
#define YOFFSET 0
#define ZOFFSET 0
#define EOFFSET 0

#define XYJERK 22
#define XACCELL 40
#define XMOVEACCELL 40

#define XMAXFEEDRATE 240
#define YMAXFEEDRATE 240
#define ZMAXFEEDRATE 240
#define E0MAXFEEDRATE 10
//106.66,213.3332,1066.6666,174.864
#define XSTEPPERMM 106.66//1780//131//178
#define YSTEPPERMM 213.3332//125//175//125
#define ZSTEPPERMM 1066.6666//1020//1020 //420
#define E0STEPPERMM 174.864//340//380

#ifndef NUMBUFFER
#define NUMBUFFER 80//20
#endif

#define XMAX 150//1200
#define YMAX 125//1800
#define ZMAX 120

#define MOTOR_X_BACKLASH 0  // MOTOR 0 = X, 1= Y 2=Z 3=E
#define MOTOR_Y_BACKLASH 0
#define MOTOR_Z_BACKLASH 0
#define MOTOR_E_BACKLASH 0

//#define AUTO_MOTOR_Z_OFF



//#define INVERTENDSTOP // uncomment for normally open

#define ENDSTOP_MOVE 3   //2mm move back after endstop hit, warning, must
#define HOMING_MOVE 2000

