/**********************************************************
 *  Puts the processor into Wait (VLPW) mode aka... Sleep. 
 *  All system interrupts will exit this sleep mode. Users
 *  must call attachInterrupt for the configured pin or have
 *  whatever interrupt handling routine.
 *
 *  This example shows using Interval Timer to wake the
 *  processor from Sleep mode. Notice the you will have
 *  to resync the clock when waking up from any Sleep modes
 *  which is done through the callback function is this
 *  example.
 *
 *  Tested and compiled under Arduino 1.0.5 and 
 *  Teensyduino 1.18
 *********************************************************/
#include <LowPower_Teensy3.h>
#include <Time.h> 

volatile uint8_t LEDPIN = 13;
uint16_t threshold;
long blinkRate = 500000;

TEENSY3_LP LP = TEENSY3_LP();
IntervalTimer timer0;

void intervalTimerCallback() {
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  setSyncProvider(getTeensy3Time);
}

void setup() {
  Serial1.begin(9600);
  setSyncProvider(getTeensy3Time);
  pinMode(LEDPIN, OUTPUT);
  /*****************************************************
   * Setup IntervalTimer to wake from Sleep mode.
   *****************************************************/
  timer0.begin(intervalTimerCallback, blinkRate);
}

void loop() {
  /*****************************************************
   * Sleep for 5 secs then print the current time.
   *****************************************************/
  LP.Sleep();
  digitalClockDisplay();
  Serial1.println(" - Wake Time");
  Serial1.flush();
}

void digitalClockDisplay() {
  Serial1.print(month());
  Serial1.print("/");
  Serial1.print(day());
  Serial1.print("/");
  Serial1.print(year());
  Serial1.print(" ");
  Serial1.print(hourFormat12());
  printDigits(minute());
  printDigits(second());
}

time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

void printDigits(int digits) {
  Serial1.print(":");
  if(digits < 10) Serial1.print('0');
  Serial1.print(digits);
}
