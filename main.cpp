
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

//i2c variables
uint8_t target = 0x68;
uint8_t who_am_i = 0x75;

//bluetooth variables
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
char alarmed = '0';
char armed = '0';
char secret = '0';
char sound = '0';
char sound_sel = '0';
char sound_delay = '0';

char debug_command = 0;
char debug_output[255];

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
	//bluetooth_init();
	
	//bluetooth_set_mode(BT_GENERAL_DISCOVERABLE, BT_UNDIRECTED_CONNECTABLE);
	
	for(;;){
		if(usb_serial_available()){
			debug_command = usb_serial_getchar();
		}
		delay(100);
		digitalWriteFast(LED_BUILTIN, LOW);
		delay(30);
		digitalWriteFast(LED_BUILTIN, HIGH);
		
		if(bt_new_data == 1){
			armed = bt_armed + ASCII;
			sound = bt_sound + ASCII;
			sound_sel = bt_sound_select + ASCII;
			sound_delay = bt_sound_delay + ASCII;
			
			bt_new_data = 0;
		}
		
		if(armed == '1' && alarmed == '0'){
			alarmed = motion_update();
		}
		if(alarmed == '1'){
			simplePrint("FREAKOUT! I'M BEING STOLEN!\n");
		}
		
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
			
			/*
			motion_get_gyro_values();
			sprintf(debug_output, "Gyro X: %f ", GYRO_XANGLE);
			simplePrint(debug_output);
			sprintf(debug_output, "Y: %f ", GYRO_YANGLE);
			simplePrint(debug_output);
			sprintf(debug_output, "Z: %f ", GYRO_ZANGLE);
			simplePrint(debug_output);
			
			sprintf(debug_output, "X_OUT: %i ", GYRO_XOUT);
			simplePrint(debug_output);
			sprintf(debug_output, "Y_OUT: %i ", GYRO_YOUT);
			simplePrint(debug_output);
			sprintf(debug_output, "Z_OUT: %i\n", GYRO_ZOUT);
			simplePrint(debug_output);
			
			motion_get_accel_values();
			sprintf(debug_output, "Accel X: %f ", ACCEL_XANGLE);
			simplePrint(debug_output);
			sprintf(debug_output, "Y: %f ", ACCEL_YANGLE);
			simplePrint(debug_output);
			sprintf(debug_output, "Z: %f ", ACCEL_ZANGLE);
			simplePrint(debug_output);
			
			sprintf(debug_output, "X_OUT: %i ", ACCEL_XOUT);
			simplePrint(debug_output);
			sprintf(debug_output, "Y_OUT: %i ", ACCEL_YOUT);
			simplePrint(debug_output);
			sprintf(debug_output, "Z_OUT: %i\n", ACCEL_ZOUT);
			simplePrint(debug_output);
			*/
			debug_command = 0;
		}
		
	}
	
	return(0);
}