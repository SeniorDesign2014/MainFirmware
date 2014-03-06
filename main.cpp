
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "mk20dx128.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "bluetooth.h"
#include "HardwareSerial.h"
//this is a common header for functions used in several c files (ex. simpleprint())
#include "common.h"		
#include "MPU6050.h"

//i2c variables TODO: make these constants
uint8_t target = 0x68;
uint8_t who_am_i = 0x75;

//bluetooth variables  TODO: move these to header
extern int bt_write_flag;
extern int bt_whitelist_flag;
extern int bt_set_mode_flag;
extern int bt_connected_flag;
extern int bt_armed;
extern int bt_sound;
extern int bt_sound_select;
extern int bt_sound_delay;
extern int bt_new_data;

//state varaibales
int state = 0;
char alarmed = '0';
char armed = '0';
char secret = '0';
char sound = '0';
char sound_sel = '0';
char sound_delay = '0';

//varaibles to debug with over serial (115200 baud)
char debug_command = 0;
char debug_output[255];

//TODO: remove this
/* DECIDED TO GO WITH SPRINTF 
void PrettyFormatInt( int i, char* buf, int buflen ){
  // don't forget ends:
  ostrstream temp( buf, buflen );
  temp << setw(4) << i << ends;
}

void PrettyFormatFloat(float i, std::string *s){
	std::ostringstream ss;
	ss << i;
	*s(ss.str());
}
*/

int main(void){
	pinMode(LED_BUILTIN, OUTPUT);
	bluetooth_init();
	
	//bluetooth_set_mode(BT_GENERAL_DISCOVERABLE, BT_UNDIRECTED_CONNECTABLE);
	
	for(;;){
	
	switch(state){ //TODO: make enumeration
	
		case 0x00: // STATE_SLEEPING	
			
			//TODO: 
			
		break;
		case 0x01: // STATE_SLEEP
			
			if(bt_new_data == 1){ //TODO: make this flag work
				armed = bt_armed + ASCII;
				sound = bt_sound + ASCII;
				sound_sel = bt_sound_select + ASCII;
				sound_delay = bt_sound_delay + ASCII;
				
				bt_new_data = 0;
			}
			
		break;
		
		case 0x10: // STATE_ARMING
			//TODO: get GPS lock
			//TODO: sleep GPS
			
			//TODO: state = STATE_ARMED;
			
		break;
		
		case: 0x11: //STATE_ARMED
		
			//poll the motion sensor
			if(alarmed == '0'){
				alarmed = motion_update();
				//TODO: state = STATE_ALARMED;
			}
			
			//TODO: check for disarm message
			//TODO: state = STATE_SLEEPING	
				
			
		break;
		
		case 0x20; //STATE_ALARMING
			
			//TODO: fire up GPS
			//TODO: fire up GSM
			
			//TODO: check for disarm message
			//TODO: state = STATE_SLEEPING
			
		break;
		
		case 0x21; //STATE_ALARMED
			simplePrint("FREAKOUT! I'M BEING STOLEN!\n");
			
			//TODO: get GPS data
			//TODO: send GSM data
			
			//TODO: check for disarm message
			//TODO: state = STATE_SLEEPING
			
		break;
		
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	//THIS IS MY MANUAL DEBUG CODE
		if(usb_serial_available()){
			debug_command = usb_serial_getchar();
		}
		delay(100);
		digitalWriteFast(LED_BUILTIN, LOW);
		delay(30);
		digitalWriteFast(LED_BUILTIN, HIGH);
		
		//bluetooth_update();
		if(debug_command == 'b'){
			simplePrint("Setting up Bluetooth.../n");
			bluetooth_set_mode(BT_GENERAL_DISCOVERABLE, BT_UNDIRECTED_CONNECTABLE);
			debug_command = 0;
		}
		if(debug_command == 'w'){
			bluetooth_write(secret, armed, sound, sound_sel, sound_delay);
			debug_command = 0;
		}
		if(debug_command == 'i'){
			bluetooth_write('0','0','0','0','0');
			debug_command = 0;
		}
		
		if(debug_command == 'm'){
		//	motion_init();
		//	motion_begin();
			motion_i2c_init();
			//Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
			simplePrint("I2C has initialized!\n");
			debug_command = 0;
		}
		
		if(debug_command == 'n'){
			
			int response = -1;
			while(response != 0){
				response = motion_init();
				if(response == 0){
					simplePrint("Everything Worked!\n");
				}else{
					simplePrint("nothing worked\n");
				}
			}
			
			motion_calibrate();

			debug_command = 0;
		}
		
		if(debug_command == 'a'){
			if(armed == '1'){
				armed = '0';
				alarmed = '0';
				simplePrint("System Disarmed.\n");
			}else{
				armed = '1';
				simplePrint("System Armed.\n");
				motion_arm_position();
			}
			debug_command = 0;
		}
		
	}
	
	return(0);
}
