#include "config_pins.h"
#include "common.h"
#include "temp.h"
#include "timer.h"
#include "motion.h"


uint32_t next_temp;
uint16_t ctemp = 0;
double Setpoint, Input, Output;
int wait_for_temp = 0;

int fan_val = 0;
void setfan_val(int val) {
#ifdef fan_pin
  pinMode(fan_pin, OUTPUT);
#ifdef usetmr1
  digitalWrite(fan_pin, val);
#else
  analogWrite(fan_pin, val);
#endif
  //  digitalWrite(fan_pin, val);
  fan_val = val;
#endif
}


#if defined(temp_pin) && !defined(ISPC)
#include <PID_v1.h>



//Specify the links and initial tuning parameters

PID myPID(&Input, &Output, &Setpoint, 8, 2, 12, DIRECT); //2, 5, 1, DIRECT);



#if defined(__AVR__) && defined(ISRTEMP)
int vanalog[8];
int adcpin;

ISR (ADC_vect)
{
  uint8_t low, high;

  // we have to read ADCL first; doing so locks both ADCL
  // and ADCH until ADCH is read.  reading ADCL second would
  // cause the results of each conversion to be discarded,
  // as ADCL and ADCH would be locked when it completed.
  low = ADCL;
  high = ADCH;

  vanalog[adcpin] = (high << 8) | low;
  /*Serial.print(adcpin);
    Serial.print(":");
    Serial.print(vanalog[adcpin]);
    Serial.write('\n');
  */
}  // end of ADC_vect
#endif

void set_temp(float set) {
  Setpoint = set;
  pinMode(heater_pin, OUTPUT);
#ifdef usetmr1
  digitalWrite(heater_pin, 0);

#else
  analogWrite(heater_pin, 0);
#endif

}
void init_temp()
{
  //initialize the variables we're linked to

  //turn the PID on

  myPID.SetMode(AUTOMATIC);

  next_temp = micros();
  set_temp(0);
#ifdef temp_pin
  pinMode(temp_pin, INPUT);
#if defined( __AVR__) && defined(ISRTEMP)

  ADCREAD(temp_pin)
#elif defined(ESP8266)
//////////////////////
////// added for esp -- under test
///////////////////////////

  analogRead(temp_pin);
  //adcvalue = analogRead(A0);
//  serial.print("VALUE: "+String(adcvalue));
  
//////////////////////
////// End  added for esp -- under test
///////////////////////////  
#endif
#endif

}

float read_temp(int32_t temp) {

  for (int j = 0; j < NUMTEMPS; j++) {
    //Serial.println(pgm_read_word(&(temptable[j][1])));
   // Serial.println(temp);
    
    if ( pgm_read_word(&(temptable[j][0])) <= temp ) {

      
      if (( pgm_read_word(&(temptable[j][0])) - temp) == 0){
        return float(pgm_read_word(&(temptable[j][1])));
      }
      else
      { float tempi =  pgm_read_word(&(temptable[j][1]))-pgm_read_word(&(temptable[j-1][1]));
        
        float tempo =  pgm_read_word(&(temptable[j-1][0]))-pgm_read_word(&(temptable[j][0]));
         tempi= (tempi/tempo)*(pgm_read_word(&(temptable[j-1][0]))- temp)+ pgm_read_word(&(temptable[j-1][1]));
         
         
        return float(tempi);
      }
     }
  }
  return 0;
}

int WindowSize = 1500;
unsigned long windowStartTime;
void temp_loop(uint32_t cm)
{
  if (cm-next_temp>TEMPTICK) {
    next_temp = cm; // each 0.5 second
    int v = 0;
#if defined( __AVR__) && defined(ISRTEMP)
    // automatic in ESR
    ADCREAD(temp_pin)
    v = vanalog[temp_pin];
#else
   // v = analogRead(temp_pin) >> ANALOGSHIFT;
    //zprintf(PSTR("%d\n"),fi(v));
#endif
#ifdef ESP8266
    v=analogRead(A0);
#endif
    Input =  read_temp(v);
#ifdef fan_pin
    if ((Input > 80) && (fan_val < 50)) setfan_val(255);
#endif
    if (Setpoint > 0) {
#ifdef heater_pin

      myPID.Compute();
#ifdef ESP8266

#ifdef usetmr1
      /************************************************
           turn the output pin on/off based on pid output
         ************************************************/
      unsigned long now = millis();
      if (now - windowStartTime > WindowSize)
      { //time to shift the Relay Window
        windowStartTime += WindowSize;
      }
      if (Output*4 > now - windowStartTime) digitalWrite(heater_pin, HIGH);
      else digitalWrite(heater_pin, LOW);
      //zprintf(PSTR("OUT:%d %W:%d\n"),fi(Output),fi(now-windowStartTime)); 
      #warning USING BANG BANG HEATER
#else
      analogWrite(heater_pin, Output * 4);
      #warning USING PWM HEATER
#endif

#elif defined __ARM__
      analogWrite(heater_pin, Output * 2);
#else
      analogWrite(heater_pin, Output * 17 / 20);
#endif
      if (wait_for_temp ) {
        //zprintf(PSTR("Temp:%f @%f\n"), ff(Input), ff(Output));
      }
#endif
    }
  }
}
int temp_achieved() {
  return Input >= Setpoint - 10;

  //  return fabs(Input - Setpoint) < 10;
}

#else

int temp_achieved() {
  return 1;
}
void set_temp(float set) {
}

void init_temp()
{
}
void temp_loop(uint32_t cm)
{
}
#endif

