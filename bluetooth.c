/**************************************
bluetooth.c
Created: 02/26/2014
Author: Paul Burris
**************************************/
#include "mk20dx128.h"
#include "core_pins.h"
#include "usb_serial.h"
#include "bluetooth.h"
#include "HardwareSerial.h"
#include "common.h"

//#define USB_DEBUG

int bt_write_flag = 0;
int bt_whitelist_flag = 0;
int bt_set_mode_flag = 0;
int bt_connected_flag = 0;
int bt_armed = 0;
int bt_sound = 0;
int bt_sound_select = 0;
int bt_sound_delay = 0;
int bt_new_data = 0; 

char bt_serial_out[BT_WRITE_BUF_SIZE] = 
	{0x0D, 0x00, 0x09, 0x02, 0x00, 0x14, 0x00, 0x00, 0x05, 0x30, 0x30, 0x30, 0x30, 0x30};
char bt_whitelist_append[BT_WHITELIST_BUF_SIZE] = 
	{0x0B , 0x00, 0x07, 0x00, 0x0A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
char bt_set_mode[BT_SET_MODE_BUF_SIZE] = {0x06, 0x00, 0x02, 0x06, 0x01, 0x02, 0x02};
	
void simplePrint(char* S){
	while(*S){
		usb_serial_putchar(*S++);
	}
}

void bluetooth_init(){
	serial2_begin(BAUD2DIV(38400));
	serial2_format(SERIAL_8N1);
	serial2_clear();
}
/* Write Attribute*/
void bluetooth_write(char handshake, char arm_disarm, char sound_on_off, char sound_sel, char sound_delay){
	bt_serial_out[9] = handshake;
	bt_serial_out[10] = arm_disarm;
	bt_serial_out[11] = sound_on_off;
	bt_serial_out[12] = sound_sel;
	bt_serial_out[13] = sound_delay;
	
	serial2_write(bt_serial_out, BT_WRITE_BUF_SIZE);
}

void bluetooth_set_mode(char discover, char connect){
	bt_set_mode[5] = discover;
	bt_set_mode[6] = connect;
	
	serial2_write(bt_set_mode, BT_SET_MODE_BUF_SIZE);
}

void bluetooth_whitelist_append(char* device_address, char address_type){
	int i;
	for(i=0; i<6; i++){
		bt_whitelist_append[i+5] = device_address[i];
	}
	bt_whitelist_append[11] = address_type;
	
	serial2_write(bt_whitelist_append, BT_WHITELIST_BUF_SIZE);
}

/**********************************************************
This function checks for the response of the Bluetooth moodule 
and should be called on a regular basis
*********************************************************/
void bluetooth_update(){
	char msg_type;
	char length;
	char class;
	char method;
	char data[32];
	char i = 0;
	
	#ifdef USB_DEBUG
	usb_serial_putchar((char)serial2_available());
	#endif
	
	if((serial2_available()>4)){
		msg_type = serial2_getchar();
		length = serial2_getchar();
		//usb_serial_putchar(length);
		class = serial2_getchar();
		method = serial2_getchar();
		
		for(i=0;i<length;i++){
			data[i] = serial2_getchar();
			if(i>30){
				break;
				serial2_clear();
			}
		}
		
		switch(msg_type){
			case 0x00: //response message
				#ifdef USB_DEBUG
				simplePrint("Response: ");
				#endif
				switch(class){
					case 0x00: //system
						switch(method){
							case 0x0A: //whitelist append
								if((data[0] & data[1]) == 0){bt_whitelist_flag = 1;}
								else{bt_whitelist_flag = -1;}
							break;
						}
					break;
					case 0x02: //attribute database
						switch(method){
							case 0x00: //write
								#ifdef USB_DEBUG
								simplePrint("Write\n");
								#endif
								if((data[0] & data[1]) == 0){bt_write_flag = 1;}
								else{bt_write_flag = -1;}
							break;
						}
					break;
					case 0x06: //GAP
						switch(method){
							case 0x01: //set mode
								#ifdef USB_DEBUG
								simplePrint("Mode Set\n");
								#endif
								if((data[0] & data[1]) == 0){bt_set_mode_flag = 1;}
								else{bt_set_mode_flag = -1;}
							break;
						}
					break;
				}
			break;

			case 0x80: //event message
				#ifdef USB_DEBUG
				simplePrint("Event: ");
				#endif
				switch(class){
					case 0x00: //Bootup
						switch(method){
							case 0x00: //bootup
								//TODO: fix
							break;
						}
					break;
					
					case 0x02: //Data
						switch(method){
							case 0x00: //status
								#ifdef USB_DEBUG
								simplePrint("New data!\n");
								#endif
								bt_new_data = 1;
								bt_armed = data[8] - ASCII;
								bt_sound = data[9] - ASCII;
								bt_sound_select = data[10] - ASCII;
								bt_sound_delay = data[11] - ASCII;
								
							break;
						}
					break;
					
					case 0x03: //Connection
						switch(method){
							case 0x00: //status
								#ifdef USB_DEBUG
								simplePrint("Connected\n");
								#endif
								if(data[0] & BT_CONNECTION_COMPLETE){bt_connected_flag = 1;}
								else{bt_connected_flag = 0;}
							break;
							
							case 0x04: //disconnected
								#ifdef USB_DEBUG
								simplePrint("Disconnected\n");
								#endif
								bt_connected_flag = 0;
							break;
						}
					break;
				}
			break;
			
			default:
				#ifdef USB_DEBUG
				simplePrint("Default\n");
				#endif
			break;
		}
	}
}