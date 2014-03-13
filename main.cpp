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
#include "secret.h"
#include "HardwareSerial.h"
#include "MPU6050.h"
#include "Tone.h"
//this is a common header for functions used in several c files ex. simpleprint
#include "common.h"		

#ifndef SECRET_NUMBER
#define SECRET_NUMBER 16668675309
#endif

//state variables
int state = 0;
char alarmed = '0';
char armed = '0';
char sound = '0';
char sound_sel = '0';
char sound_delay = '0';
char sound_test = '0';

enum states {
	STATE_DISARMING,
	STATE_DISARMED,
	STATE_ARMING,
	STATE_ARMED,
	STATE_ALARMING,
	STATE_ALARMED
};

//variables for GSM
char gsm_message[255] = "";
int gsm_counter = 460;
int text_lock = 2;

//variables for GPS
int gps_lock = -1;
int gps_ret = -1;
int gps_index = 0;
struct location gps_loc;
struct location gps_temp;

//variable for motion
char motion_redundancy[MOTION_REDUNDANCY_COUNT];
int motion_index = 0;
int motion_i = 0; 
char motion_check = 0;

//variables for bluetooth
int bt_broadcasting = 0;

//variables for audio
int audio_time_elapsed = 0;
int audio_test_time = 20;
uint16_t audio_frequency[5] = {500, 1000, 1500, 2500, 5000};
int audio_delay[] = {0, 240, 480, 720}; 
int audio_pattern_count = 0;
int audio_pattern[5][4] = {{250, 250, 250, 250}, {500, 500, 500, 500}, {250, 250, 250, 0}, {250, 250, 250, 250}, {300, 100, 300, 100}};
int audio_timeout = 0;

//variables to use usb serial debugging (115200 baud)
char debug_command = 0;
char debug_output[255];
int led_toggle = 1;

int main(void){

	pinMode(LED_BUILTIN, OUTPUT);

	//perform start up
	bluetooth_init();
	motion_init();
	motion_calibrate();
	tone_init();

	bluetooth_reset();
	delay(250);
	bluetooth_update();

	//TODO: implement bluetooth whitleist
	

	for(;;){
		if(led_toggle){
			digitalWriteFast(LED_BUILTIN, LOW);
			led_toggle = 0;
		} else{
			digitalWriteFast(LED_BUILTIN, HIGH);
			led_toggle = 1;
		}

		//poll bluetooth every so often
		bluetooth_update();
		if(bt_new_data == 1){
			armed = bt_armed;
			sound = bt_sound;
			sound_sel = bt_sound_select;
			sound_delay = bt_sound_delay;
			sound_test = bt_sound_test;
			
			bluetooth_write(sound_test, armed, sound, sound_sel, sound_delay);
			bt_new_data = 0;
		}

		//state machine
		switch(state){
		
			case STATE_DISARMING:
				simplePrint("DISARMING\n");
				gps_end();
				gsm_end();
				if(sound_test == '0'){
					tone_end();
					audio_time_elapsed = 0;
				}
				alarmed = '0';
				state = STATE_DISARMED;
				
			break;
			case STATE_DISARMED:
				simplePrint("DISARMED\n");
				if(armed == '1'){state = STATE_ARMING;}
				
				if(sound_test == '1'){
					simplePrint("TESTING SOUND: ");
					usb_serial_putchar(sound_sel);

					tone_begin(audio_frequency[sound_sel - ASCII], audio_pattern[sound_sel][audio_pattern_count]);
					audio_pattern_count++;
					if(audio_pattern_count == 4){ audio_pattern_count = 0;}

					audio_test_time--;
					if(audio_test_time == 0){
						audio_test_time = 20;
						tone_end();
						sound_test = '0';
						bluetooth_write(sound_test, armed, sound, sound_sel, sound_delay);
					}
				}

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
					//get GPS data
					gps_ret = gps_parse(&gps_temp);
					if(gps_ret == 1){
						gps_lock = 1;
						for(gps_index=0;gps_index<4;gps_index++){ gps_loc.lat[gps_index] = gps_temp.lat[gps_index];}
						for(gps_index=0;gps_index<8;gps_index++){ gps_loc.lat_min[gps_index] = gps_temp.lat_min[gps_index];}
						for(gps_index=0;gps_index<5;gps_index++){ gps_loc.lon[gps_index] = gps_temp.lon[gps_index];}
						for(gps_index=0;gps_index<8;gps_index++){ gps_loc.lon_min[gps_index] = gps_temp.lon_min[gps_index];}
						simplePrint("valid location - ");
					}else if(gps_ret < 0){
						gps_lock = 0;	
						simplePrint("invalid - ");
					}

					if(gps_lock > 0){
						gps_sleep();
						simplePrint("GPS locked and sleeping\n");
					}
				}
				
				//TODO: timer (with int)
				delay(250);
				
				//poll the motion sensor every .25 s
				motion_redundancy[motion_index] = motion_update();
				motion_index++;
				if(motion_index == MOTION_REDUNDANCY_COUNT){
					motion_index = 0;
				}
				motion_check = 0;
				for(motion_i=0; motion_i<MOTION_REDUNDANCY_COUNT; motion_i++){
					if('1' == motion_redundancy[motion_i]){
						motion_check++;
					}
				}
				if(motion_check == MOTION_REDUNDANCY_COUNT){
					alarmed = '1';
				}
				if(alarmed == '1'){
					state = STATE_ALARMING;
				}
				
				if(armed == '0'){state = STATE_DISARMING;}
			break;
			
			case STATE_ALARMING:
				simplePrint("ALARMING\n");
				gps_wake();
				
				if(gsm_init(0) != 1){
					simplePrint("ERROR - GSM did not init. Have a nice day.\n");
				}
				
				state = STATE_ALARMED;
				
			break;
			
			case STATE_ALARMED:
				simplePrint("ALARMED\n");
				
				//get GPS data
				gps_ret = gps_parse(&gps_temp);
				if(gps_ret == 1){
					gps_lock = 1;
					for(gps_index=0;gps_index<4;gps_index++){ gps_loc.lat[gps_index] = gps_temp.lat[gps_index];}
					for(gps_index=0;gps_index<8;gps_index++){ gps_loc.lat_min[gps_index] = gps_temp.lat_min[gps_index];}
					for(gps_index=0;gps_index<5;gps_index++){ gps_loc.lon[gps_index] = gps_temp.lon[gps_index];}
					for(gps_index=0;gps_index<8;gps_index++){ gps_loc.lon_min[gps_index] = gps_temp.lon_min[gps_index];}
					simplePrint("valid location - ");

				}else if(gps_ret == 2){
					for(gps_index=0;gps_index<6;gps_index++){ gps_loc.vel[gps_index] = gps_temp.vel[gps_index];}
					simplePrint("valid velocity - ");
				}else if(gps_ret < 0){
					gps_lock = 0;	
					simplePrint("invalid - ");
				}

				//send GSM data every 2 min 
				if(gsm_counter >= 480){ 
					simplePrint("I'll be texting you shortly; ");
					//if data is valid, cleverly and secretly pack the message
					//TODO: put in timeout
					if(gps_lock){
						simplePrint(gps_loc.lat);
						simplePrint(gps_loc.lat_min);
						simplePrint(gps_loc.lon);
						simplePrint(gps_loc.lon_min);
						simplePrint(gps_loc.vel);
						
						gps_pack_message(gsm_message, &gps_loc, alarmed);
						simplePrint("packed you a secret message.\n");
						simplePrint(gsm_message);
						if(text_lock > 0){
							text_lock--;
							gsm_send_sms(SECRET_NUMBER, gsm_message);
						}
						gsm_counter = 0; //reset counter
					}else{
						simplePrint("No GPS lock\n");
					}

				}else{ gsm_counter++;}

				//audio (with correct settings)
				if(sound == '1'){
					simplePrint("PLAYING SOUND");
					if(audio_time_elapsed >= audio_delay[sound_delay - ASCII]){
						tone_begin(audio_frequency[sound_sel - ASCII], audio_pattern[sound_sel][audio_pattern_count]);
						audio_pattern_count++;
						if(audio_pattern_count == 4){ audio_pattern_count = 0;}

						audio_timeout++;
						if(audio_timeout >= 1200){
							audio_timeout = 0;
							sound = '0';
						}
					}else{
						audio_time_elapsed++;
					}
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
		/*
		
		if(debug_command == 'b'){
			simplePrint("Setting up Bluetooth.../n");
			bluetooth_set_mode(BT_GENERAL_DISCOVERABLE, BT_UNDIRECTED_CONNECTABLE);
			debug_command = 0;
		}
		
		*/

		if(debug_command == 't'){
			sound_test = '1';
			debug_command = 0;
		}

		if(debug_command == '4'){
			sound_sel = '4';
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
