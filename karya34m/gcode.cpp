#include "gcode.h"
#include "common.h"
#include "motion.h"
#include "config_pins.h"
#include "timer.h"
#include "temp.h"
#include "motors.h"
#include "eprom.h"


int32_t linecount, lineprocess;
#ifdef USETIMER1
#define MLOOP
#else
#define MLOOP coreloopm();
#endif

#include<stdint.h>
#ifndef ISPC
#include<arduino.h>
#endif
#include "eprom.h"
/// crude crc macro
#define crc(a, b)    (a ^ b)
decfloat read_digit;
int sdcardok = 0;
int waitforline = 0;
char g_str[40];
int g_str_c = 0;

GCODE_COMMAND next_target;
uint16_t last_field = 0;
/// list of powers of ten, used for dividing down decimal numbers for sending, and also for our crude floating point algorithm

static float decfloat_to_float(void)
{
  float r = read_digit.mantissa;
  uint8_t e = read_digit.exponent;
  /*  uint32_t powers=1;
    for (e=1; e<read_digit.exponent;e++) powers*=10;
    // e=1 means we've seen a decimal point but no digits after it, and e=2 means we've seen a decimal point with one digit so it's too high by one if not zero
  */
  if (e) r = (r /*+ powers[e-1] / 2*/) / POWERS(e - 1);
  //  if (e) r = (r /*+ powers[e-1] / 2*/) * POWERS(e - 1);
  MLOOP
  return read_digit.sign ? -r : r;
}
void pausemachine()
{
  PAUSE = !PAUSE;
  if (PAUSE)zprintf(PSTR("Pause\n"));
  else zprintf(PSTR("Resume\n"));
}
void changefilament(float l)
{
#ifdef CHANGEFILAMENT
  waitbufferempty();
  float backupE = ce01;
  float backupX = cx1;
  float backupY = cy1;
  float backupZ = cz1;

  addmove(50, 0, 0, 0, -2, 0, 1); // retract
  addmove(50, 0, 0, 30, 0, 0, 1); // move up
  addmove(50, 0, 0, 0, -l, 0, 1); // unload filament
  waitbufferempty();
  checkendstop = 1;
  //zprintf(PSTR("change filemant, then push endstop\n"));
  while (1) {
    docheckendstop();
    if (endstopstatus < 0) break;
    domotionloop

  }
  checkendstop = 0;
  addmove(5, 0, 0, 0, l + 10, 0, 1); // load filament
  addmove(50, 0, 0, -30, 0, 0, 1);
  waitbufferempty();
  ce01 = backupE;
  cx1 = backupX;
  cy1 = backupY;
  cz1 = backupZ;
#endif
}

uint8_t gcode_parse_char(uint8_t c)
{
  uint8_t checksum_char = c;
  //Serial.write(c);
  // uppercase
  if (c >= 'a' && c <= 'z' && !next_target.read_string)
    c &= ~32;

  // An asterisk is a quasi-EOL and always ends all fields.
  if (c == '*') {
    //next_target.read_string = 0;
  }

  // Skip comments and strings.
  if (
    next_target.read_string == 0
  ) {
    // Check if the field has ended. Either by a new field, space or EOL.
    if (last_field && (c < '0' || c > '9') && c != '.') {
      switch (last_field) {
        case 'G':
          next_target.G = read_digit.mantissa;
          break;
        case 'M':
          next_target.M = read_digit.mantissa;
          if (next_target.M == 117) next_target.read_string = 1;

          break;
        case 'X':

          next_target.target.axis[X] = decfloat_to_float();
          break;
        case 'Y':
          next_target.target.axis[Y] = decfloat_to_float();
          break;
        case 'Z':
          next_target.target.axis[Z] = decfloat_to_float();
          break;
#ifdef ARC_SUPPORT
        case 'I':
          next_target.I = decfloat_to_float();
          break;
        case 'J':
          next_target.J = decfloat_to_float();
          break;
        case 'R':
          next_target.R = decfloat_to_float();
          break;
#endif
        case 'E':
          next_target.target.axis[E] = decfloat_to_float();
          break;
        case 'F':
          // just use raw integer, we need move distance and n_steps to convert it to a useful value, so wait until we have those to convert it
          next_target.target.F = decfloat_to_float() / 60;
          MLOOP
          break;
        case 'S':
          next_target.S = decfloat_to_float();
          break;
        case 'P':
          next_target.P = decfloat_to_float();
          break;
        case '*':
          //next_target.checksum_read = decfloat_to_float();
          break;
        case 'T':
          //next_target.T = read_digit.mantissa;
          break;
        case 'N':
          //next_target.N = decfloat_to_float();
          break;

      }
    }

    // new field?
    if ((c >= 'A' && c <= 'Z') || c == '*') {
      last_field = c;
      read_digit.sign = read_digit.mantissa = read_digit.exponent = 0;
    }

    // process character
    // Can't do ranges in switch..case, so process actual digits here.
    // Do it early, as there are many more digits than characters expected.
    if (c >= '0' && c <= '9') {
      if (read_digit.exponent < DECFLOAT_EXP_MAX + 1) {
        // this is simply mantissa = (mantissa * 10) + atoi(c) in different clothes
        read_digit.mantissa = (read_digit.mantissa << 3) +
                              (read_digit.mantissa << 1) + (c - '0');
        if (read_digit.exponent)
          read_digit.exponent++;
      }
    } else {
      switch (c) {
        // Each currently known command is either G or M, so preserve
        // previous G/M unless a new one has appeared.
        // FIXME: same for T command
        case 'G':
          next_target.seen_G = 1;
          //next_target.seen_M = 0;
          //next_target.M = 0;
          break;
        case 'M':
          next_target.seen_M = 1;
          //next_target.seen_G = 0;
          //next_target.G = 0;
          break;
#ifdef ARC_SUPPORT
        case 'I':
          next_target.seen_I = 1;
          break;
        case 'J':
          next_target.seen_J = 1;
          break;
        case 'R':
          next_target.seen_R = 1;
          break;
#endif
        case 'X':
          next_target.seen_X = 1;
          break;
        case 'Y':
          next_target.seen_Y = 1;
          break;
        case 'Z':
          next_target.seen_Z = 1;
          break;
        case 'E':
          next_target.seen_E = 1;
          break;
        case 'F':
          next_target.seen_F = 1;
          break;
        case 'S':
          next_target.seen_S = 1;
          break;
        case 'P':
          next_target.seen_P = 1;
          break;

        case 'T':
          next_target.seen_T = 1;
          break;
        case 'N':
          //next_target.seen_N = 1;
          break;
        case '*':
          //next_target.seen_checksum = 0;//1;
          break;
        // comments
        case '[':
          next_target.read_string = 1;  // Reset by ')' or EOL.
          g_str_c = 0;
          break;

        case ';':
          next_target.read_string = 1;   // Reset by EOL.
          break;
        // now for some numeracy
        case '-':
          read_digit.sign = 1;
          // force sign to be at start of number, so 1-2 = -2 instead of -12
          read_digit.exponent = 0;
          read_digit.mantissa = 0;
          break;
        case '.':
          if (read_digit.exponent == 0)
            read_digit.exponent = 1;
          break;
#ifdef  DEBUG
        case ' ':
        case '\t':
        case 10:
        case 13:
          // ignore
          break;
#endif

        default:
#ifdef  DEBUG
          // invalid
          //zprintf(PSTR("?%d\n"), fi(c));
#endif
          break;
      }
    }
  } //else if ( next_target.seen_parens_comment == 1 && c == ')')
  else {
    // store string in g_str  from gcode example M206 P450 [ryan widi]
    if (c == ']') {
      g_str[g_str_c] = 0;
      next_target.read_string = 0;
    } else {
      g_str[g_str_c] = c;
      g_str_c++;
    }
    //next_target.seen_parens_comment = 0; // recognize stuff after a (comment)
  }


  // end of line
  if ((c == 10) || (c == 13)) {

    // Assume G1 for unspecified movements.
    /*    if ( ! next_target.seen_G && ! next_target.seen_M && ! next_target.seen_T &&
             (next_target.seen_X || next_target.seen_Y || next_target.seen_Z ||
              next_target.seen_E || next_target.seen_F)) {
          next_target.seen_G = 1;
          next_target.G = 1;
        }
    */
    MLOOP
    process_gcode_command();

    // reset variables
    uint8_t ok = next_target.seen_G || next_target.seen_M || next_target.seen_T;
    next_target.seen_X = next_target.seen_Y = next_target.seen_Z = \
                         next_target.seen_E = next_target.seen_F = next_target.seen_S = \
                             next_target.seen_P = next_target.seen_T = \
                                 next_target.seen_G = next_target.seen_M = \
                                     next_target.read_string = g_str_c  = 0;
    MLOOP
#ifdef ARC_SUPPORT
    next_target.seen_R = next_target.seen_I = next_target.seen_J = 0;
#endif
    last_field = 0;
    read_digit.sign = read_digit.mantissa = read_digit.exponent = 0;

    if (next_target.option_all_relative) {
      next_target.target.axis[X] = next_target.target.axis[Y] = next_target.target.axis[Z] = 0;
    }
    if (next_target.option_all_relative || next_target.option_e_relative) {
      next_target.target.axis[E] = 0;
    }
    if (ok) return 2;
    return 1;
  }

  return 0;
}


// implement minimalis code to match teacup

float lastE;

void printposition()
{
  zprintf(PSTR("X:%f Y:%f Z:%f E:%f\n"),
          ff(cx1), ff(cy1),
          ff(cz1), ff(ce01));

}

#define queue_wait() needbuffer()

void delay_ms(uint32_t d)
{
#ifndef ISPC
  while (d) {
    d--;
    somedelay(1000);
  }
#else
  // delay on pc,

#endif

}
void temp_wait(void)
{

#ifdef heater_pin
  wait_for_temp = 1;
  int c = 0;
  while (wait_for_temp && !temp_achieved()) {
    domotionloop
    servo_loop();
    if (c++ > 20000) {
      c = 0;
      zprintf(PSTR("T:%f\n"), ff(Input));
      //zprintf(PSTR("Heating\n"));
    }
  }
  wait_for_temp = 0;
#endif

}
//int32_t mvc = 0;
static void enqueue(GCODE_COMMAND *) __attribute__ ((always_inline));

inline void enqueue(GCODE_COMMAND *t, int g0 = 1)
{
  amove(t->target.F, t->seen_X ? t->target.axis[X] : cx1
        , t->seen_Y ? t->target.axis[Y] : cy1
        , t->seen_Z ? t->target.axis[Z] : cz1
        , t->seen_E ? t->target.axis[E] : ce01
        , g0);
}
inline void enqueuearc(GCODE_COMMAND *t, float I, float J, int cw)
{
  draw_arc(t->target.F, t->seen_X ? t->target.axis[X] : cx1
           , t->seen_Y ? t->target.axis[Y] : cy1
           , t->seen_Z ? t->target.axis[Z] : cz1
           , t->seen_E ? t->target.axis[E] : ce01
           , I, J, cw);
}

void process_gcode_command()
{
  uint32_t  backup_f;

  // convert relative to absolute
  if (next_target.option_all_relative) {
    next_target.target.axis[X] += cx1;//startpoint.axis[X];
    next_target.target.axis[Y] += cy1;//startpoint.axis[Y];
    next_target.target.axis[Z] += cz1;//startpoint.axis[Z];
    next_target.target.axis[E] += ce01;//startpoint.axis[Z];
  }

  // E relative movement.
  // Matches Sprinter's behaviour as of March 2012.
  if (next_target.option_all_relative || next_target.option_e_relative)
    next_target.target.e_relative = 1;
  else
    next_target.target.e_relative = 0;

  // The GCode documentation was taken from http://reprap.org/wiki/Gcode .

  if (next_target.seen_T) {
    //? --- T: Select Tool ---
    //?
    //? Example: T1
    //?
    //? Select extruder number 1 to build with.  Extruder numbering starts at 0.

    //next_tool = next_target.T;
  }
  // check if buffer is near full
  /*
    int bl=bufflen();
    float spd=1;
    if (bl>NUMBUFFER/2) {
      spd=(float)NUMBUFFER/(bl*4+2);
    }
  */
  if (next_target.seen_G) {
    uint8_t axisSelected = 0;
    //zprintf(PSTR("Gcode %su \n"),next_target.G);
    switch (next_target.G) {
      case 0:
        //? G0: Rapid Linear Motion
        //?
        //? Example: G0 X12
        //?
        //? In this case move rapidly to X = 12 mm.  In fact, the RepRap firmware uses exactly the same code for rapid as it uses for controlled moves (see G1 below), as - for the RepRap machine - this is just as efficient as not doing so.  (The distinction comes from some old machine tools that used to move faster if the axes were not driven in a straight line.  For them G0 allowed any movement in space to get to the destination as fast as possible.)
        //?
        enqueue(&next_target, 1);
        break;

      case 1:
        //? --- G1: Linear Motion at Feed Rate ---
        //?
        //? Example: G1 X90.6 Y13.8 E22.4
        //?
        //? Go in a straight line from the current (X, Y) point to the point (90.6, 13.8), extruding material as the move happens from the current extruded length to a length of 22.4 mm.
        //?
        //next_target.target.axis[E]=0;
        // auto retraction change

        // thread S parameter as value of the laser, in 3D printer, donot use S in G1 !!
        if (next_target.seen_S) {
          //laserOn = next_target.S > 0;
          //constantlaserVal = next_target.S;
        }
        enqueue(&next_target, 0);
        break;

      //  G2 - Arc Clockwise
      //  G3 - Arc anti Clockwise
      case 2:
      case 3:
#ifdef ARC_SUPPORT
        temp_wait();

        if (!next_target.seen_I) next_target.I = 0;
        if (!next_target.seen_J) next_target.J = 0;
        //if (DEBUG_ECHO && (debug_flags & DEBUG_ECHO))

        enqueuearc(&next_target, next_target.I, next_target.J, next_target.G == 2);
#endif
        break;

      case 4:
        //? --- G4: Dwell ---
        //?
        //? Example: G4 P200
        //?
        //? In this case sit still doing nothing for 200 milliseconds.  During delays the state of the machine (for example the temperatures of its extruders) will still be preserved and controlled.
        //?
        queue_wait();
        // delay
        if (next_target.seen_P) {
          zprintf(PSTR("\n\nPause enabled:next_target.P-%d\n\n"), next_target.P);

       // the next statement, takes  around 100-110ms to get executed. we will aproximate it as 100ms 
       // #### MLOOP  might be the cause of such a big delay
          for (; next_target.P > 0; next_target.P--) {//
            MLOOP
            delay_ms(5); // 
            //zprintf(PSTR("Pause 5ms\n"));
            zprintf(PSTR("Pause:P-%d\n\n"), next_target.P);
          }
        }
        break;
#ifdef output_enable
      case 5:
        reset_eeprom();
        reload_eeprom();
      case 6:
        cx1 = 0;
        cy1 = 0;
        cz1 = 0;
        ce01 = 0;
        amove(1, 100, 100, 100, 0);
        /*
          amove(100, 10, 0, 0, 0);
          amove(100, 10, 10, 0, 0);
          amove(100, 0, 10, 0, 0);
          amove(100, 0, 0, 0, 0);

          amove(100, 10, 0, 0, 0);
          amove(100, 10, 10, 0, 0);
          amove(100, 0, 10, 0, 0);
          amove(100, 0, 0, 0, 0);

          amove(100, 10, 0, 0, 0);
          amove(100, 10, 10, 0, 0);
          amove(100, 0, 10, 0, 0);
          amove(100, 0, 0, 0, 0);

          amove(100, 10, 0, 0, 0);
          amove(100, 10, 10, 0, 0);
          amove(100, 0, 10, 0, 0);
          amove(100, 0, 0, 0, 0);
        */
        break;
#endif
      case 7: // baby step in S in milimeter
        int nstep;
        /*if (next_target.seen_X) move_motor(0, sx[0], next_target.target.axis[X] * stepmmx[0]);
          if (next_target.seen_Y) move_motor(1, sx[1], next_target.target.axis[Y] * stepmmx[1]);
          if (next_target.seen_Z) move_motor(2, sx[2], next_target.target.axis[Z] * stepmmx[2]);
          if (next_target.seen_S) {
          move_motor(0, sx[0], next_target.S * stepmmx[0]);
          move_motor(1, sx[1], next_target.S * stepmmx[1]);
          move_motor(2, sx[2], next_target.S * stepmmx[2]);
          }

          float bx,by,bz;
          bx=cx1;
          by=cy1;
          bz=cz1;
          if (next_target.seen_Z) addmove(50,bx,by,next_target.Z+bz,ce01);
          cx1=bx;
          cy1=by;
          cz1=bz;*/

        break;
      case 28:
        homing();
        next_target.target.axis[X] = cx1;
        next_target.target.axis[Y] = cy1;
        next_target.target.axis[Z] = cz1;
        next_target.target.axis[E] = ce01;
        printposition();
        break;

      case 90:
        //? --- G90: Set to Absolute Positioning ---
        //?
        //? Example: G90
        //?
        //? All coordinates from now on are absolute relative to the origin
        //? of the machine. This is the RepRap default.
        //?
        //? If you ever want to switch back and forth between relative and
        //? absolute movement keep in mind, X, Y and Z follow the machine's
        //? coordinate system while E doesn't change it's position in the
        //? coordinate system on relative movements.
        //?

        // No wait_queue() needed.
        next_target.option_all_relative = 0;
        break;

      case 91:
        //? --- G91: Set to Relative Positioning ---
        //?
        //? Example: G91
        //?
        //? All coordinates from now on are relative to the last position.
        //?

        // No wait_queue() needed.
        next_target.option_all_relative = 1;
        break;

      case 92:
        //? --- G92: Set Position ---
        //?
        //? Example: G92 X10 E90
        //?
        //? Allows programming of absolute zero point, by reseting the current position to the values specified.  This would set the machine's X coordinate to 10, and the extrude coordinate to 90. No physical motion will occur.
        //?

        queue_wait();

        if (next_target.seen_X) {
          cx1 = next_target.target.axis[X];
          axisSelected = 1;
        };
        if (next_target.seen_Y) {
          cy1 = next_target.target.axis[Y];
          axisSelected = 1;
        };
        if (next_target.seen_Z) {
          cz1 = next_target.target.axis[Z];
          axisSelected = 1;
        };
        if (next_target.seen_E) {
          lastE = ce01 = next_target.target.axis[E];
          axisSelected = 1;
        };

        if (axisSelected == 0) {
          cx1 = next_target.target.axis[X] =
                  cy1 = next_target.target.axis[Y] =
                          cz1 = next_target.target.axis[Z] =
                                  ce01 = next_target.target.axis[E] = 0;
        }
        init_pos();

        break;

      // unknown gcode: spit an error
      default:
        zprintf(PSTR("E:G%d\nok\n"), next_target.G);
        return;
    }
  } else if (next_target.seen_M) {
    //uint8_t i;

    switch (next_target.M) {
      /*#ifndef ISPC
            case 200: // keybox action
              if (next_target.seen_P) {

                zprintf(PSTR("DOKEY:%d\n"), next_target.P);
                switch (next_target.P) {
                    KBOX_DO_ACT
                }
              }
              break;
        #endif
      */
      case 0:
      //? --- M0: machine stop ---
      //?
      //? Example: M0
      //?
      //? http://linuxcnc.org/handbook/RS274NGC_3/RS274NGC_33a.html#1002379
      //? Unimplemented, especially the restart after the stop. Fall trough to M2.
      //?

      case 2:
        // stop and clear all buffer
        if (m) {
          RUNNING = 0;
          waitbufferempty();
          printposition();
        }
        break;
      case 25:
        // stop and clear all buffer
        pausemachine();
        break;

      case 84: // For compatibility with slic3rs default end G-code.
        //? --- M2: program end ---
        //?
        //? Example: M2
        //?
        //? http://linuxcnc.org/handbook/RS274NGC_3/RS274NGC_33a.html#1002379
        //?
        queue_wait();
        //for (i = 0; i < NUM_HEATERS; i++)temp_set(i, 0);
        power_off();

        zprintf(PSTR("\nstop\n"));
        break;

      /*      case 6:
              //? --- M6: tool change ---
              //?
              //? Undocumented.
              //tool = next_tool;
              break;
      */
      // M3/M101- extruder on M3 S -> PWM output to heated pin
      // M4 are special, usually to make spindle counter clockwise and we dont have implementation, but we use it for laser
      // mode : constant laser burn per step
      // M4 Sxxx   0: disable 1:enable
      // if defined, M3 Sxxx, xxx will define how manu microseconds laser will on on each step.
      // xxx will limit the G1 maximum feedrate
      //

      case 4:
        // already implemented using value = 255 = cutting
        //constantlaser = next_target.S == 1;
        break;
      case 5:
        // already implemented using value = 255 = cutting
        //constantlaser = next_target.S == 1;
        next_target.seen_S=1;
        next_target.S=0;
      case 3:
#ifdef LASERMODE
        // if no S defined then full power
        if (!next_target.seen_S)next_target.S = 255;
        laserOn = next_target.S > 0;
        constantlaserVal = next_target.S;
        if (laserOn) zprintf(PSTR("LASERON\n"));
        if (next_target.seen_P) {
          if (m)waitbufferempty();
          delay(100);
          //pinMode(laser_pin, OUTPUT);
          zprintf(PSTR("PULSE LASER\n"));
          digitalWrite(laser_pin, HIGH);
          delay(next_target.P);
        }
        digitalWrite(laser_pin, LOW);

#endif
        break;
        /*      case 101:
                //? --- M101: extruder on ---
                //?
                //? Undocumented.
                temp_wait();
                // enable the laser or spindle
                break;

              // M5/M103- extruder off
              case 5:
              case 103:
                //? --- M103: extruder off ---
                //?
                //? Undocumented.
                // disable laser/spindle
                break;
        */
#ifdef servo_pin
      case 300:
        waitbufferempty();
        setfan_val(255); // turn on power
        zprintf(PSTR("Servo:%d\n"), fi(next_target.S));
        //pinMode(servo_pin,OUTPUT);
        servo_set(next_target.S * (2000 / 180));
        if (!next_target.seen_P)next_target.P = 1000; // 1 second wait
        // wait loop
        uint32_t mc;
        mc = millis();
        while ((millis() - mc) < next_target.P) {
          domotionloop
        }
        setfan_val(0); // turn off power
        break;
#endif
      case 104:
        set_temp(next_target.S);
        break;
      case 105:
        zprintf(PSTR("T:%f\n"), ff(Input));
        //zprintf(PSTR("B:%d/%d\n"), fi(bufflen()),fi(NUMBUFFER));
        break;
      case 109:
        set_temp(next_target.S);
        temp_wait();
        break;
      case 7:
      case 107:
        // set laser pwm off
#ifdef fan_pin
        setfan_val(0);
#endif
        break;
      case 106:
        // set laser pwm on
#ifdef fan_pin
        setfan_val(next_target.S);
#endif
        break;

      case 112:
        //? --- M112: Emergency Stop ---
        //?
        //? Example: M112
        //?
        //? Any moves in progress are immediately terminated, then the printer
        //? shuts down. All motors and heaters are turned off. Only way to
        //? restart is to press the reset button on the master microcontroller.
        //? See also M0.
        //?
        // stop and clear all buffer
        RUNNING = 0;

        break;

      case 114:
        //? --- M114: Get Current Position ---
        //?
        //? Example: M114
        //?
        //? This causes the RepRap machine to report its current X, Y, Z and E coordinates to the host.
        //?
        //? For example, the machine returns a string such as:
        //?
        //? <tt>ok C: X:0.00 Y:0.00 Z:0.00 E:0.00</tt>
        //?
#ifdef ENFORCE_ORDER
        // wait for all moves to complete
        queue_wait();
#endif
        printposition();
        break;

      case 115:
        //        zprintf(PSTR("FIRMWARE_NAME:Repetier_1.9 FIRMWARE_URL:null PROTOCOL_VERSION:1.0 MACHINE_TYPE:teacup EXTRUDER_COUNT:1 REPETIER_PROTOCOL:\n"));
        zprintf(PSTR("FIRMWARE_NAME:Repetier_1.9\n"));

        break;


      case 119:
        //? --- M119: report endstop status ---
        //? Report the current status of the endstops configured in the
        //? firmware to the host.
        docheckendstop();
        zprintf(PSTR("END:"));
        zprintf(endstopstatus < 0 ? PSTR("HI ") : PSTR("LOW "));

        zprintf(PSTR("\n"));

        break;

        // unknown mcode: spit an error
#ifdef USE_EEPROM
#ifndef SAVE_RESETMOTION
      case 502:
        reset_eeprom();
#endif
      case 205:
        reload_eeprom();
#endif
      case 503:
        zprintf(PSTR("EPR:3 145 %f Xhome\n"), ff(ax_home[0]));
        zprintf(PSTR("EPR:3 149 %f Y\n"), ff(ax_home[1]));
        zprintf(PSTR("EPR:3 153 %f Z\n"), ff(ax_home[2]));

        zprintf(PSTR("EPR:3 3 %f StepX\n"), ff(stepmmx[0]));
        zprintf(PSTR("EPR:3 7 %f Y\n"), ff(stepmmx[1]));
        zprintf(PSTR("EPR:3 11 %f Z\n"), ff(stepmmx[2]));
        zprintf(PSTR("EPR:3 0 %f E\n"), ff(stepmmx[3]));

        zprintf(PSTR("EPR:2 15 %d FeedX\n"), fi(maxf[0]));
        zprintf(PSTR("EPR:2 19 %d Y\n"), fi(maxf[1]));
        zprintf(PSTR("EPR:2 23 %d Z\n"), fi(maxf[2]));
        zprintf(PSTR("EPR:2 27 %d E\n"), fi(maxf[3]));


        zprintf(PSTR("EPR:3 181 %d Jerk\n"), fi(xyjerk));
        zprintf(PSTR("EPR:3 51 %d Accel\n"), fi(accel));
        zprintf(PSTR("EPR:3 67 %d MVAccel\n"), fi(mvaccel));
        zprintf(PSTR("EPR:3 177 %d FHome\n"), fi(homingspeed));
        zprintf(PSTR("EPR:3 185 %f XYscale\n"), ff(xyscale));
#ifdef NONLINEAR
        zprintf(PSTR("EPR:3 157 %f RodL\n"), ff(delta_diagonal_rod));
        zprintf(PSTR("EPR:3 161 %f RodH\n"), ff(delta_radius));
#endif
        zprintf(PSTR("EPR:3 165 %f Xofs\n"), ff(axisofs[0]));
#ifdef DRIVE_XYYZ
        zprintf(PSTR("EPR:3 169 %f Y1ofs\n"), ff(axisofs[1]));
        zprintf(PSTR("EPR:3 173 %f Y2ofs\n"), ff(axisofs[2]));
#else
        zprintf(PSTR("EPR:3 169 %f Yofs\n"), ff(axisofs[1]));
        zprintf(PSTR("EPR:3 173 %f Zofs\n"), ff(axisofs[2]));
#endif
#ifdef USE_BACKLASH
        zprintf(PSTR("EPR:3 80 %f Xbcklsh\n"), fi(xback[0]));
        zprintf(PSTR("EPR:3 84 %f Y\n"), fi(xback[1]));
        zprintf(PSTR("EPR:3 88 %f Z\n"), fi(xback[2]));
        zprintf(PSTR("EPR:3 92 %f E\n"), fi(xback[3]));
#endif

        break;
#ifdef WIFISERVER
      // show wifi
      case 504:
        zprintf(PSTR("Wifi AP 400:%s PWD 450:%s mDNS 470:%s telID 380:%s\n"), wifi_ap, wifi_pwd, wifi_dns, wifi_telebot);
        break;
#endif
#ifdef USE_EEPROM
      case 206:
        if (next_target.seen_X)next_target.S = next_target.target.axis[X];
        int32_t S_F;
        S_F = (next_target.S * 1000);
        int32_t S_I;
        S_I = (next_target.S);
        if (next_target.seen_P)
          switch (next_target.P) {
#define eprom_wr(id,pos,val){\
  case id:\
    eepromwrite(pos, val);\
    break;\
  }
              eprom_wr(145, EE_xhome, S_F);
              eprom_wr(149, EE_yhome, S_F);
              eprom_wr(153, EE_zhome, S_F);
              eprom_wr(0, EE_estepmm, S_F);
              eprom_wr(3, EE_xstepmm, S_F);
              eprom_wr(7, EE_ystepmm, S_F);
              eprom_wr(11, EE_zstepmm, S_F);

              eprom_wr(15, EE_max_x_feedrate, S_I);
              eprom_wr(19, EE_max_y_feedrate, S_I);
              eprom_wr(23, EE_max_z_feedrate, S_I);
              eprom_wr(27, EE_max_e_feedrate, S_I);


              eprom_wr(51, EE_accelx, S_I);

              eprom_wr(67, EE_mvaccelx, S_I);
              eprom_wr(177, EE_homing, S_I);
              eprom_wr(181, EE_jerk, S_I);
              eprom_wr(185, EE_xyscale, S_F);
#ifdef NONLINEAR
              eprom_wr(157, EE_rod_length, S_F);
              eprom_wr(161, EE_hor_radius, S_F);
#endif
              eprom_wr(165, EE_towera_ofs, S_F);
              eprom_wr(169, EE_towerb_ofs, S_F);
              eprom_wr(173, EE_towerc_ofs, S_F);

#ifdef USE_BACKLASH
              eprom_wr(80, EE_xbacklash, S_F);
              eprom_wr(84, EE_ybacklash, S_F);
              eprom_wr(88, EE_zbacklash, S_F);
              eprom_wr(92, EE_ebacklash, S_F);
#endif
#ifdef WIFISERVER
            case 380:
              eepromwritestring(380, g_str);
              break;
            case 400:
              eepromwritestring(400, g_str);
              break;
            case 450:
              eepromwritestring(450, g_str);
              break;
            case 470:
              eepromwritestring(470, g_str);
              break;
#endif
          }
        reload_eeprom();
        break;
#endif
      case 220:
        //? --- M220: Set speed factor override percentage ---
        if ( ! next_target.seen_S)
          break;
        // Scale 100% = 100
        f_multiplier = next_target.S * 0.01;
        MLOOP

        break;

      case 221:
        //? --- M220: Set speed factor override percentage ---
        if ( ! next_target.seen_S)
          break;
        // Scale 100% = 256
        e_multiplier = next_target.S * 0.01;
        MLOOP

        break;
      case 600: // change filament M600 Sxxx          S = length mm to unload filament, it will add 10mm when load, click endstop to resume
        changefilament(next_target.S);
        break;
        //      default:
        //zprintf(PSTR("E:M%d\nok\n"), next_target.M);
    } // switch (next_target.M)
  } // else if (next_target.seen_M)
} // process_gcode_command()

void init_gcode()
{
  next_target.target.F = 50;
  next_target.option_all_relative = 0;
#ifdef USE_EEPROM
  eeprominit
#endif

}

