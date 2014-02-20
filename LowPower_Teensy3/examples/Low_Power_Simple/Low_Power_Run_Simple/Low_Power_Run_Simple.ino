/*************************************************************  
 *   Puts the processor into Very Low Power Run Mode
 *  (VLPR). In this mode, all clocks are enabled but 
 *  reduced to Core(2MHZ), BUS(2MZ), Flash(1MHZ). 
 *
 *  This example uses IntervalTimer to show Teensy3 
 *  going in and out of VLPR Mode. While in VLPR mode
 *  the clocks are at reduced speed so USB is disabled. 
 *  Teensy3 will not be able to be programed normally 
 *  while in VLPR mode.
 *   
 *  Tested and compiled under Arduino 1.0.5 and 
 *  Teensyduino 1.16rc1.    
 ************************************************************/
#include <LowPower_Teensy3.h>

volatile uint8_t LEDPIN = 13;
long blinkRate = 50000;

TEENSY3_LP LP = TEENSY3_LP();
IntervalTimer timer0;

void intervalTimerCallback() {
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
}

void setup() {
  pinMode(LEDPIN, OUTPUT);
  
  // loop through five times
  for (uint8_t i = 0; i < 5; i++) {
    
    /****************************************************
     * Setup IntervalTimer to toggle led every 50ms in 
     * Normal Run Mode, clocks at F_CPU.
     ****************************************************/
    timer0.begin(intervalTimerCallback, blinkRate);
    delay(5000);
    
    /****************************************************
     * Go into Very Low Power Run Mode. Notice the led 
     * will now blink slower because the Teensy3 is now
     * clocked at 2MHZ.
     ****************************************************/
    LP.Run(LP_RUN_ON);
    delay(5000);
    
    /****************************************************
     * Reconfigure IntervalTimer period for 2MHZ clock.
     * Now the led will toggle every 50ms.
     ****************************************************/
    timer0.begin(intervalTimerCallback, 
                 intervalTimerHandler(blinkRate));
    delay(5000);
    
    /****************************************************
     * Stop IntervalTimer in VLPR mode and pause for 2 
     * secs.
     ****************************************************/
    timer0.end();
    digitalWrite(LEDPIN, LOW);
    delay(5000);
    
    /****************************************************
     * Move into Normal Run Mode. Clock now at F_CPU.
     ****************************************************/
    LP.Run(LP_RUN_OFF);
  }
}
void loop() {
  // Once here programing resumes as normal
}

float intervalTimerHandler(float period) {
  period = period/(F_BUS/2000000);
  return period;
}





