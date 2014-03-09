/**********************************************************
 *  Puts the processor into Wait (VLPW) mode aka... Sleep. 
 *  All system interrupts will exit this sleep mode. Users
 *  must call attachInterrupt for the configured pin or have
 *  whatever interrupt handling routine.
 *
 *  This example shows using the digital pin 0 as wakeup
 *  source.  
 *
 *  Tested and compiled under Arduino 1.0.5 and 
 *  Teensyduino 1.18 
 *********************************************************/
#include <LowPower_Teensy3.h>

volatile uint8_t LEDPIN = 13;

TEENSY3_LP LP = TEENSY3_LP();

void callbackhandler() {
  digitalWrite(LEDPIN, !digitalRead(LEDPIN));
}

void setup() {
  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);
  pinMode(0, INPUT_PULLUP);
  attachInterrupt(0, callbackhandler, RISING);
}

void loop() {
  /*****************************************************
   * Setup wake source as Digital Pin 0, using 
   * attachInterrupt function.
   *****************************************************/
  LP.Sleep();
}
