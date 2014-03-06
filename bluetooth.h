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
 
#define BT_WRITE_BUF_SIZE 14
#define BT_WHITELIST_BUF_SIZE 12
#define BT_SET_MODE_BUF_SIZE 7

#define BT_NON_DISCOVERABLE 0
#define BT_LIMITED_DISCOVERABLE 1
#define BT_GENERAL_DISCOVERABLE 2 //recommended

#define BT_NON_CONNECTABLE 0
#define BT_DIRECTED_CONNECTABLE 1
#define BT_UNDIRECTED_CONNECTABLE 2 //recommended

#define BT_CONNECTION_COMPLETE 4

#define ASCII 0x30

//Call this first!
void bluetooth_init();

//Then call this, please, with the reccommended modes ;^)
void bluetooth_set_mode(char discover, char connect);

//This function is used to put up data into the advertised output characteristic
void bluetooth_write(char handshake, char arm_disarm, char sound_on_off, char sound_sel, char sound_delay);

//Only append when you are not connected
void bluetooth_whitelist_append(char* device_address, char address_type);

 #ifdef __cplusplus
 }
 #endif

#endif
