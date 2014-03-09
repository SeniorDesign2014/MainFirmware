//Tone header file

#include "core_pins.h"
#include "pins_arduino.h"
#include "HardwareSerial.h"
#include "IntervalTimer.h"

void tone_interrupt(void);

void tone(uint8_t pin, uint16_t frequency, uint32_t duration);

void noTone(uint8_t pin);