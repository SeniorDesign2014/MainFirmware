/*This is the main file of the Teensy.*/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "./teensy3/mk20dx128.h"
#include "./teensy3/HardwareSerial.h"	//arduino style c functions to use serial
#include "./teensy3/util/delay.h"	//mapping from avr style delay to 
#include "./teensy3/core_pins.h"
#include "./teensy3/avr_emulation.h"
#include "./teensy3/arm_math.h"

int main(void){
/*simple Hello World by flipping a pin (and turning on and off an led)*/
//Setup pin PORTC.5 pin
//Individual pin configuration
PORTC_PCR5 = 0x00000140;

//GPIO Direction Register
GPIOC_PDDR = 0x00000020;

while(1){
	//Toggle pin C5
//	GPIOC_PDOR = 0x00000020;
	GPIOC_PSOR = 0x00000020;
	delay(200);
	GPIOC_PCOR = 0x00000000;
	delay(500);

}
}//End Main
