
#ifdef BOARD

/*
  ============================================================================================
     _K34M_Plasma
  ============================================================================================
*/
#elif defined(BOARD_K34M_Plasma)


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

#define xdirection 14 //D5
#define xstep 2 //D4 //2
#define ydirection 13 //D7
#define ystep 0 //D3

// z and e have same direction pin, we think that normally slicer never move z and e together.. we hope we are right :D
#define zdirection 12// D6
#define zstep 16 // D0
#define e0direction 12 // D6
#define e0step 4 //D2

#define limit_pin 15 //D8

//#define temp_pin A0
//#define heater_pin D1//5 //D1
//#define laser_pin D1
#define fan_pin D1 // To be used for plama relay/contact

//#define INVERTENDSTOP
#define NUMBUFFER 60 // increased buffer from 20 to 80

#else
#error No BOARD Defined !
#endif
