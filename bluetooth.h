/**************************************
bluetooth.h
Created: 02/27/2014
Author: Paul Burris
**************************************/
#ifndef BLUETOOTH_H
#define BLUETOOTH_H

 #ifdef __cplusplus
 extern "C" {
 #endif
 
#define BT_NON_DISCOVERABLE 0
#define BT_LIMITED_DISCOVERABLE 1 //lower power, I think
#define BT_GENERAL_DISCOVERABLE 2 //recommended

#define BT_NON_CONNECTABLE 0
#define BT_DIRECTED_CONNECTABLE 1 	//TODO: look into this
#define BT_UNDIRECTED_CONNECTABLE 2 //recommended

#define BT_CONNECTION_COMPLETE 4

#define ASCII 0x30

//bluetooth variables
extern char bt_write_flag;
extern char bt_whitelist_flag;
extern char bt_set_mode_flag;
extern char bt_connected_flag;
extern char bt_armed;
extern char bt_sound;
extern char bt_sound_select;
extern char bt_sound_delay;
extern char bt_new_data;

//Call this first!
void bluetooth_init(void);

//call this to check for new messages and update flags
void bluetooth_update(void);

//Then call this, please, with the reccommended modes ;^)
void bluetooth_set_mode(char discover, char connect);

//This function is used to put up data into the advertised output characteristic
void bluetooth_write(char handshake, char arm_disarm, char sound_on_off, char sound_sel, char sound_delay);

//Only append when you are not connected
void bluetooth_whitelist_append(char* device_address, char address_type);

void bluetooth_reset(void);

 #ifdef __cplusplus
 }
 #endif

#endif
