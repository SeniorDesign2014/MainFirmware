//Tone header file

#include "core_pins.h"
#include "pins_arduino.h"
#include "HardwareSerial.h"
#include "IntervalTimer.h"

//call this first
void tone_init(void);

//Wakes amp and starts the tone
void tone_begin(uint16_t frequency, uint32_t duration);

//sleeps the amp and stops tone
void tone_end(void);

//isr called when interrupt triggers
void tone_interrupt(void);

//called in tone begin
void tone(uint8_t pin, uint16_t frequency, uint32_t duration);

//if you want to change tones, call noTone then tone
void noTone(uint8_t pin);
