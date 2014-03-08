/*
	gsm.h
	
	Firmware for Teensy3.1
	Interface with GSM Click module
	
	Russell Barnes, Paul Burris, Nick Voigt 2014
*/

#include <stdio.h>
#include "mk20dx128.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "HardwareSerial.h"

// Accept up to this many characters from the GSM module for each command:
#define RECEIVE_LIMIT 150
// Time to wait for a response from the GSM module before a timeout and command failure, in seconds:
#define COMMAND_TIMEOUT 10


/*
	****** PUBLIC FUNCTIONS ******
*/


/*
	gsm_init

	A blocking function that should be called before any of the other functions
	in this file.
	Calls gsm_init, which prints data to the USB serial interface.
	This function can safely be called multiple times.

	Return values:
	1: success
	0: timeout (no response from GSM module)
	-1: Unexpected response from GSM module
*/
uint8_t gsm_init(void);

/*
	gsm_send_sms

	Send an SMS message to the specified number.
	Messages should be limited to 260 characters.
	Ensure that gsm_init() has been called before this function.

	phonenumber:
	Null-terminated string in the format of 15031112222

	message:
	Null-terminated string less than or equal to 260 in length.

	Return values:
	1: success
	0: timeout (no response from GSM module)
	-1: Unexpected response from GSM module
*/
uint8_t gsm_send_sms(char *phonenumber, char *message);




/*
	****** HELPER FUNCTIONS ******
*/




/*
	gsm_write

	A blocking function that sends a command to the GSM module.
	Prints the result to the USB serial interface.

	Return values:
	1: success
	0: timeout (no response from GSM module)
	-1: Unexpected response from GSM module
*/
uint8_t gsm_write(char *cmd, int len);

/*
	gsm_verify_response

	A blocking function that compares the received string to known successful
	responses from the GSM Click module.
	Received does not have to be null-terminated.

	Return values:
	1: success
	0: Unexpected response from GSM module
*/
uint8_t gsm_verify_response(char *received);

/*
	USBPrint
	
	A debug function to print the null-terminated string s to the 
	Teensy's USB interface.
*/
void USBPrint(char *s);