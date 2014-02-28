/*#include <stdio.h> 
#include <stdlib.h>
#include <string.h>*/
#include "mk20dx128.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "HardwareSerial.h"


void TestPrint(char* S){
	while(*S){
		usb_serial_putchar(*S++);
	}
}

uint8_t gsm_init(void)
{
uint8_t success;

// Write several noop commands
serial_write("AT\r", 3);
delay(250);
serial_write("AT\r", 3);
delay(250);
serial_write("AT\r", 3);
delay(250);
// Clear the serial buffer and write one more noop
serial_clear();
// We are expecting to receive "OK"
success = gsm_write("AT\r", 3);


}

uint8_t gsm_write(char *cmd, int len)
{
uint8_t available;

delay(100);
serial_write(cmd, len);
delay(300);
available = serial_available();
if (!available) 
	return 0;

}

int main(void)
{
int success;

pinMode(LED_BUILTIN, OUTPUT);

// Initialize serial
serial_begin(BAUD2DIV(115200));
serial_format(SERIAL_8N1);

// Initialize GSM module
gsm_init();

while(1){
	digitalWriteFast(LED_BUILTIN, HIGH);
	delay(500);
	digitalWriteFast(LED_BUILTIN, LOW);
	delay(500);

	TestPrint("This is a test! Hi World!\n\r");
	
	serial_write("This is UART!\r\n", 15);
	//serial_putchar('n');
}

return(0);
}
