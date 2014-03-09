#include tone.h


/*
	Sets the output of the sleep pin
*/
void tone_init(void){
	pinMode(20, OUTPUT);
	digitalWriteFast(20, LOW);
}

/*
	Function wakes up audio amp, passes frequency and duration to the tone generation
	function. No returns.
*/

void tone_begin(uint16_t frequency, uint32_t duration){
	digitalWriteFast(20, HIGH);
	tone(14, frequency, duration);
}

/*
	Calls noTone to stop noise and sleeps audio amp.
*/
void tone_end(void){
	digitalWriteFast(20, LOW);
	noTone(14);
}
