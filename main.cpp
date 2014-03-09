/**********************************
main.cpp
Created: 03/06/2014
Authors: Paul Burris, Nick Voigt, Russell Barnes

This is the integrated file.
**********************************/
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include "mk20dx128.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "bluetooth.h"
#include "gsm.h"
#include "gps.h"
#include "HardwareSerial.h"
#include "MPU6050.h"
//this is a common header for functions used in several c files ex. simpleprint
#include "common.h"		

//state variables
int state = 0;
char alarmed = '0';
char armed = '0';
char secret = '0';
char sound = '0';
char sound_sel = '0';
char sound_delay = '0';
enum states {
	STATE_DISARMING,
	STATE_DISARMED,
	STATE_ARMING,
	STATE_ARMED,
	STATE_ALARMING,
	STATE_ALARMED
};

//variables for GSM
char gsm_message[255] = "\0";
char gsm_phone_number[12] = "15037290820";
int gsm_counter = 0;
int text_lock = 1;

//variables for GPS
int gps_lock = -1;
struct location gps_loc;

//variables to use usb serial debugging (115200 baud)
char debug_command = 0;
char debug_output[255];

int main(void){
	pinMode(LED_BUILTIN, OUTPUT);
	bluetooth_init();
	motion_i2c_init();
	while(motion_init());
	motion_calibrate();
	
	//bluetooth_set_mode(BT_GENERAL_DISCOVERABLE, BT_UNDIRECTED_CONNECTABLE);
	
	for(;;){
		simplePrint(".");
		switch(state){
		
			case STATE_DISARMING:
				simplePrint("DISARMING\n");
				gps_end();
				gsm_end();
				//TODO: shut down audio
				state = STATE_DISARMED;
				
			break;
			case STATE_DISARMED:
				simplePrint("DISARMED\n");
				//poll bluetooth every so often
				bluetooth_update();
				if(bt_new_data == 1){
					armed = bt_armed + ASCII;
					sound = bt_sound + ASCII;
					sound_sel = bt_sound_select + ASCII;
					sound_delay = bt_sound_delay + ASCII;
					
					bt_new_data = 0;
				}
				if(armed == '1'){state = STATE_ARMING;}
				
				//TODO: sleep micro
				delay(250);

			break;
			
			case STATE_ARMING:
				simplePrint("ARMING\n");
				motion_arm_position();
				gps_init();
				state = STATE_ARMED;
			break;
			
			case STATE_ARMED:
				simplePrint("ARMED\n");

				//poll GPS until lock has been established
				if(gps_lock <= 0){
					gps_lock = gps_parse(&gps_loc);
					if(gps_lock > 0){
						gps_sleep();
					}
				}
				
				//TODO: timer (with int)
				delay(250);
				
				//poll the motion sensor every .25 s
				alarmed = motion_update();
				if(alarmed == '1'){
					state = STATE_ALARMING;
				}
				
				//poll bluetooth every so often
				bluetooth_update();
				if(bt_new_data == 1){
					armed = bt_armed + ASCII;
					sound = bt_sound + ASCII;
					sound_sel = bt_sound_select + ASCII;
					sound_delay = bt_sound_delay + ASCII;
					
					bt_new_data = 0;
				}
				if(armed == '0'){state = STATE_DISARMING;}
			break;
			
			case STATE_ALARMING:
				simplePrint("ALARMING\n");
				gps_wake();
				
				if(gsm_init() != 1){
					simplePrint("ERROR - GSM did not init. Have a nice day.\n");
				}
				
				state = STATE_ALARMED;
				
			break;
			
			case STATE_ALARMED:
				simplePrint("ALARMED\n");
				
				gps_lock = gps_parse(&gps_loc); //get GPS data

				//send GSM data every 2 min 
				if(gsm_counter >= 480){ 
					if(text_lock){
						simplePrint("I'll be texting you shortly.");
						//TODO: if data is valid, cleverly and secretly pack the message
						sprintf(gsm_message, "I am being stolen.");
						gsm_send_sms(gsm_phone_number, gsm_message);
						gsm_counter = 0; //reset counter
						text_lock = 0;
					}
				}else{ gsm_counter++;}

				//TODO: audio (with correct settings)
				
				//poll bluetooth for disarm message
				bluetooth_update();
				if(bt_new_data == 1){
					armed = bt_armed + ASCII;
					sound = bt_sound + ASCII;
					sound_sel = bt_sound_select + ASCII;
					sound_delay = bt_sound_delay + ASCII;
					
					bt_new_data = 0;
				}
				if(armed == '0'){state = STATE_DISARMING;}

				//TODO: sleep. BUT don't remove delay til then!
				//otherwise we use all our texts up super fast
				delay(250);
				
			break;
			
			default:
				simplePrint("ERROR - INCORRECT STATE\n");
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
		if(debug_command == '0'){
			bt_armed = 1;
			bt_new_data = 1;
			debug_command = 0;
		}
		if(debug_command == '1'){
			bt_armed = 0;
			bt_new_data = 1;
			debug_command = 0;
		}
		
	}
	
	return(0);
}
