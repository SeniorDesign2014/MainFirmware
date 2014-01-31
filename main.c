/*This is the main file of the Teensy.*/

#include mk20dx128.h
#include './teensy3/HardwareSerial.h'

int main(void){

char buf[]="This is a buff";

//Intialize serial hardware
serial_begin(TODO);
//Set serial data format
serial_format();

serial_write(buf, sizeof(buf));

serial_end();

}//End Main
