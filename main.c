#include "gsm.h"

/*
	gsm_example.c

	Example code for gsm.c

	This code should be run simultaneously to or after 
	powering on the GSM Click module.

	Uses Serial1 on the Teensy3.1
*/

int main(void)
{
	int success;
	
	pinMode(LED_BUILTIN, OUTPUT);
	
	// Initialize Serial1
	serial_begin(BAUD2DIV(115200));
	serial_format(SERIAL_8N1);
	
	// Initialize GSM module
	success = gsm_init();
	
	// Send an SMS message
	if (success > 0)
		success = gsm_send_sms("1503#######", "Sent from the Teensy.");
	/*
	Alternative method: 
	if (success > 0)
		success = gsm_write("AT+CMGF=1\r\n", 11);
	if (success > 0)
		success = gsm_write("AT+CMGS=\"1503#######\"\r\n", 23);
	if (success > 0)
		success = gsm_write("Sent from the Teensy\x1a", 21);
	*/
	
	while(1){
		digitalWriteFast(LED_BUILTIN, HIGH);
		delay(500);
		digitalWriteFast(LED_BUILTIN, LOW);
		delay(500);
		
		if (success > 0)
			USBPrint("SMS message transmission succeeded\r\n");
	}
	
	return(0);
}
