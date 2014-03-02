#include <stdio.h> 
//#include <stdlib.h>
//#include <string.h>
#include "mk20dx128.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "HardwareSerial.h"

// Accept up to this many characters from the GSM module for each command:
#define RECEIVE_LIMIT 150
// Time to wait for a response from the GSM module before a timeout and command failure, in seconds:
#define COMMAND_TIMEOUT 10


void USBPrint(char* s){
	while(*s){
		usb_serial_putchar(*s++);
	}
}

/*
	gsm_write

	A blocking function that sends a command to the GSM module.
	Prints the result to the USB serial interface.

	Return values:
	1: success
	0: timeout (no response from GSM module)
	-1: Unexpected response from GSM module
*/
uint8_t gsm_write(char *cmd, int len)
{
	uint8_t character_counter, timeout_seconds;
	char received[RECEIVE_LIMIT + 1];
	char print_status[RECEIVE_LIMIT + len - 1 + 39];
	char cmd_clean[len - 1];
	char c;
	
	/* 
		Clear the serial buffer for Serial1 of data before sending the command
	*/
	serial_clear();
	
	/* 
		Make a "cleaned-up" version of the command without the
		\r\n on the end for debug printing:
	*/
	snprintf(cmd_clean, len - 1, cmd);

	/* 
		Send the command and wait for the response.
		Time-out if it takes more than COMMAND_TIMEOUT seconds.
	*/
	serial_write(cmd, len);	// Write the command to the GSM module
	timeout_seconds = COMMAND_TIMEOUT;
	while (!serial_available() && timeout_seconds > 0) {
		delay(1000);//DEBUG
		timeout_seconds--;
	}	
	if (!serial_available()) {
		// No response
		snprintf(print_status, RECEIVE_LIMIT + len - 1 + 39, "No response (timed out) for command: %s\r\n", cmd_clean);
		USBPrint(print_status);
		return 0;
	}
	
	/*
		Get response from serial buffer
	*/
	character_counter = 0;
	while (serial_available() && character_counter < RECEIVE_LIMIT) {
		c = serial_getchar();
		sprintf(received + character_counter, "%c", c);
		character_counter++;
	}
	
	/*
		Parse for "OK" which is the module's success message
	*/
	if (received[0] != 'O' || received [1] != 'K') {
		// Unexpected response
		snprintf(print_status, RECEIVE_LIMIT + len - 1 + 37, "Unexpected response for command %s: %s\r\n", cmd_clean, received);
		USBPrint(print_status);
		return 0;
	}
	
	/*
		Print result to USB serial
	*/
	snprintf(print_status, RECEIVE_LIMIT + len - 1 + 26, "Response for command %s: %s\r\n", cmd_clean, received);
	USBPrint(print_status);
	return 1;
}

uint8_t gsm_init(void)
{
	// Write several noop commands
	serial_write("AT\r\n", 4);
	delay(250);
	serial_write("AT\r\n", 4);
	delay(250);
	serial_write("AT\r\n", 4);
	delay(250);
	// Write one more noop; we are expecting to receive "OK"
	return gsm_write("AT\r\n", 4);
}

int main(void)
{
	int success;

	pinMode(LED_BUILTIN, OUTPUT);

	// Initialize serial
	serial_begin(BAUD2DIV(115200));
	serial_format(SERIAL_8N1);

	// Initialize GSM module
	//gsm_init();
	
	// Send set-up commands

	while(1){
		digitalWriteFast(LED_BUILTIN, HIGH);
		delay(500);
		digitalWriteFast(LED_BUILTIN, LOW);
		delay(500);

		success = gsm_init();
		delay(1500);
	}

	return(0);
}
