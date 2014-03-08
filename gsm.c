/*
	gsm.c
	
	Firmware for Teensy3.1
	Interface with GSM Click module
	
	Russell Barnes, Paul Burris, Nick Voigt 2014
*/

#include "gsm.h"


/*
	****** PUBLIC FUNCTIONS ******
*/


/*
	gsm_init

	A blocking function that should be called before any of the other functions
	in this file.
	Instantiates serial interface 3.
	Calls gsm_init, which prints data to the USB serial interface.
	This function can safely be called multiple times.

	Return values:
	1: success
	0: timeout (no response from GSM module)
	-1: Unexpected response from GSM module
*/
uint8_t gsm_init(void)
{
	//Activate serial interface
	serial3_begin(BAUD2DIV(115200));
	serial_format(SERIAL_8N1);
	// Wait for GSM module to initialize
	delay(1000);
	// Write several noop commands
	serial3_write("AT\r\n", 4);
	delay(500);
	serial3_write("AT\r\n", 4);
	delay(500);
	serial3_write("AT\r\n", 4);
	delay(500);
	serial3_write("AT\r\n", 4);
	// Disable command echoing
	serial3_write("ATE0\r\n", 6);
	serial3_flush();
	delay(3000);
	// Enable verbose errors in case of trouble:
	//gsm_write("AT+CMEE=2\r\n", 11);
	
	// Write one more noop; we are expecting to receive "OK"
	return gsm_write("AT\r\n", 4);
}

/*
	gsm_exit

	A funtion to close the serial 3 interface

	Return Values:
	None.
*/
void  gsm_end(void){
	serial3_end();
}

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
uint8_t gsm_send_sms(char *phonenumber, char *message)
{
	uint8_t success;
	int size;
	char cmd1[24];
	char cmd2[260 + 2];
	char loopchar;
	
	
	// Create strings for message commands
	snprintf(cmd1, 24, "AT+CMGS=\"%s\"\r\n", phonenumber);
	snprintf(cmd2, 262, "%s\x1a", message);
	
	// Find length of message command
	size = 0;
	while (cmd2[size] || size >= 261) {
		size++;
	}
		
	// Send the commands via UART1
	success = gsm_write("AT+CMGF=1\r\n", 11);
	if (success > 0)
		success = gsm_write(cmd1, 23);
	if (success > 0)
		success = gsm_write(cmd2, size);
	
	return success;
}





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
	serial3_clear();
	
	/* 
		Make a "cleaned-up" version of the command without the
		\r\n on the end for debug printing:
	*/
	snprintf(cmd_clean, len - 1, cmd);

	/* 
		Send the command and wait for the response.
		Time-out if it takes more than COMMAND_TIMEOUT seconds.
	*/
	serial3_write(cmd, len);	// Write the command to the GSM module
	serial3_flush();
	timeout_seconds = COMMAND_TIMEOUT;
	while (!serial3_available() && timeout_seconds > 0) {
		delay(1000);//DEBUG
		timeout_seconds--;
	}	
	if (!serial3_available()) {
		// No response
		snprintf(print_status, RECEIVE_LIMIT + len - 1 + 39, "No response (timed out) for command: %s\r\n", cmd_clean);
		USBPrint(print_status);
		return 0;
	}
	
	/*
		Get response from serial3 buffer
	
		The first two characters will likely be \r\n!
	*/
	character_counter = 0;
	while (serial3_available() && character_counter < RECEIVE_LIMIT) {
		c = serial3_getchar();
		sprintf(received + character_counter, "%c", c);
		character_counter++;
	}
	
	/*
		Parse for "OK" which is the module's success message
	*/
	if (!gsm_verify_response(received)){
		// Unexpected response
		snprintf(print_status, RECEIVE_LIMIT + len - 1 + 37, "Unexpected response for command %s: %s\r\n", cmd_clean, received);
		USBPrint(print_status);
		serial3_clear();
		return 0;
	}
	
	/*
		Print result to USB serial
	*/
	snprintf(print_status, RECEIVE_LIMIT + len - 1 + 26, "Response for command %s: %s\r\n", cmd_clean, received);
	USBPrint(print_status);
	serial3_clear();
	return 1;
}

/*
	gsm_verify_response

	A blocking function that compares the received string to known successful
	responses from the GSM Click module.
	Received does not have to be null-terminated.

	Return values:
	1: success
	0: Unexpected response from GSM module
*/
uint8_t gsm_verify_response(char *received)
{
	/* 
		Returns 1 for success, 0 for failure.
		
		The following responses are expected: 
		OK
		#MWI: 1,1
	*/
	
	// The first two chars of the response are \r\n, so they are skipped
	return (
		(received[2] == 'O' && 
			received [3] == 'K') // "OK" - Generic success message
		||
		(received[2] == '#' && 
			received[3] == 'M' &&
			received[4] == 'W' &&
			received[5] == 'I' &&
			received[6] == ':' &&
			received[7] == ' ') // "#MWI: " - Initial boot status
		||
		(received [2] == '>' &&
			received [3] == ' ') // "> " - SMS prompt
		||
		(received[2] == '+' && 
			received[3] == 'C' &&
			received[4] == 'M' &&
			received[5] == 'G' &&
			received[6] == 'S' &&
			received[7] == ':' &&
			received[8] == ' ') // "+CMGS: ##" - Successful SMS transmission (## is the text number)
		);
}

/*
	USBPrint
	
	A debug function to print the null-terminated string s to the 
	Teensy's USB interface.
*/
void USBPrint(char *s)
{
	while(*s){
		usb_serial_putchar(*s++);
	}
}
