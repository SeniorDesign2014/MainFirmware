#include "mk20dx128.h"
#include "core_pins.h"
#include "usb_serial.h"


void TestPrint(char* S){
	while(*S){
		usb_serial_putchar(*S++);
	}
}

int main(void)
{

pinMode(LED_BUILTIN, OUTPUT);

while(1){
digitalWriteFast(LED_BUILTIN, HIGH);
delay(30);
digitalWriteFast(LED_BUILTIN, LOW);
delay(3000);

TestPrint("This is a test! Hi World!\n\r");
}

return(0);

}

