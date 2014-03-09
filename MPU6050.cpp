/**************************
MPU6050.c
Created: 03/01/2014
Author: Paul Burris
Based on the example code at https://github.com/big5824/Quadrocopter
****************************/
#include "MPU6050.h"
#include "common.h"
#include <math.h>
#include "i2c_t3.h"

#define gyro_xsensitivity 66.5 
#define gyro_ysensitivity 66.5 
#define gyro_zsensitivity 65.5
#define a 0.01

signed int ACCEL_XOUT;
signed int ACCEL_YOUT;
signed int ACCEL_ZOUT;
float GYRO_XRATE;
float GYRO_YRATE;
float GYRO_ZRATE;
int GYRO_XRATERAW;
int GYRO_YRATERAW;
int GYRO_ZRATERAW;

unsigned char GYRO_XOUT_L;
unsigned char GYRO_XOUT_H;
unsigned char GYRO_YOUT_L;
unsigned char GYRO_YOUT_H;
unsigned char GYRO_ZOUT_L;
unsigned char GYRO_ZOUT_H;
signed int GYRO_XOUT;
signed int GYRO_YOUT;
signed int GYRO_ZOUT;	

unsigned char ACCEL_XOUT_L;
unsigned char ACCEL_XOUT_H;
unsigned char ACCEL_YOUT_L;
unsigned char ACCEL_YOUT_H;
unsigned char ACCEL_ZOUT_L;
unsigned char ACCEL_ZOUT_H;

signed long GYRO_XOUT_OFFSET_1000SUM;
signed long GYRO_YOUT_OFFSET_1000SUM;
signed long GYRO_ZOUT_OFFSET_1000SUM;
signed long GYRO_XOUT_OFFSET;
signed long GYRO_YOUT_OFFSET;
signed long GYRO_ZOUT_OFFSET;

float GYRO_XANGLE;
float GYRO_YANGLE;
float GYRO_ZANGLE;
long GYRO_XANGLERAW;
long GYRO_YANGLERAW;
long GYRO_ZANGLERAW;
float ACCEL_XANGLE;
float ACCEL_YANGLE;
float ACCEL_ZANGLE;

//these are the variables for preserving the ARMED position
float ACCEL_XANGLE_ARMED;
float ACCEL_YANGLE_ARMED;
float ACCEL_ZANGLE_ARMED;

float GYRO_XANGLE_ARMED;
float GYRO_YANGLE_ARMED;
float GYRO_ZANGLE_ARMED;

void LDByteWriteI2C(unsigned char ControlByte, unsigned char Address, unsigned char data){
	
	Wire.beginTransmission(ControlByte);	//generate start condition
	Wire.write(Address);						//write address to write data to
	Wire.write(data);						//write data
	Wire.endTransmission(I2C_STOP);   		// blocking write
	Wire.finish();									//Generate Stop
	//simplePrint("I'm writing!\n");
}

void LDByteReadI2C(unsigned char ControlByte, unsigned char Address, unsigned char *Data, unsigned char Length){
	
	Wire.beginTransmission(ControlByte); 	//Generate Start Condition
	Wire.write(Address);					//Write register address to read data from
	Wire.endTransmission(I2C_NOSTOP);   	// blocking write (NOSTOP triggers RepSTART
	Wire.requestFrom(ControlByte,Length,I2C_STOP);  //Write control byte for read
	Wire.finish();									//Generate Stop
	//delay(1);
	
	if(Wire.available()){
		int i;
		for(i=0;i<Length;i++){
			Data[i] = (char)(Wire.readByte()); //read Length number of bytes
		}
	}
}

int motion_init(void){	
	int ret = 1;
	
	Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
	while(ret){	
		if(MPU6050_Test_I2C()){simplePrint("MPU is alive\n");}
		Setup_MPU6050();
		delay(10);
		ret = MPU6050_Check_Registers();
	}
	return(ret);
}

void motion_arm_position(void){
	Get_Gyro_Rates();
	GYRO_XANGLE_ARMED = GYRO_XANGLE;
	GYRO_YANGLE_ARMED = GYRO_YANGLE;
	GYRO_ZANGLE_ARMED = GYRO_ZANGLE;
	
	Get_Accel_Values();
	Get_Accel_Angles(); // TODO: accel Z angle doesn't currently work
	
	ACCEL_XANGLE_ARMED = ACCEL_XANGLE;
	ACCEL_YANGLE_ARMED = ACCEL_YANGLE;
	ACCEL_ZANGLE_ARMED = ACCEL_ZANGLE;
}

void motion_calibrate(void){
	Calibrate_Gyros();
	simplePrint("Gyros calibrated\n");

}

void motion_end(void){
	
}

/********************************************************************
This is the primary implementation for the motion sensor. It checks the
gyroscope and accelerometer for changing values. If the values have changed
beyond the starting threshold, it will return ASCII 1.
********************************************************************/
char motion_update(){
	float accelXdiff;
	float accelYdiff;
	float accelZdiff;
	float gyroXdiff;
	float gyroYdiff;
	float gyroZdiff;
	
	//check accelerometer against initial values
	Get_Accel_Values();
	Get_Accel_Angles();
	accelXdiff = ACCEL_XANGLE - ACCEL_XANGLE_ARMED;
	accelYdiff = ACCEL_YANGLE - ACCEL_YANGLE_ARMED;
	//TODO: fix
	//accelZdiff = ACCEL_ZANGLE - ACCEL_ZANGLE_ARMED;
	
	if((fabsf(accelXdiff) > ACCEL_X_TOLERANCE)|| (fabsf(accelYdiff) > ACCEL_Y_TOLERANCE)){ 
		//	|| (fabsf(accelZdiff) > ACCEL_Z_TOLERANCE)){
		return('1'); 	//something accelerometery is going on
	}
	
	//check gyroscope against initial values
	Get_Gyro_Rates();
	gyroXdiff = GYRO_XANGLE - GYRO_XANGLE_ARMED;
	gyroYdiff = GYRO_YANGLE - GYRO_YANGLE_ARMED;
	gyroZdiff = GYRO_ZANGLE - GYRO_ZANGLE_ARMED;
	
	if((fabsf(gyroXdiff) > GYRO_X_TOLERANCE)|| (fabsf(gyroYdiff) > GYRO_Y_TOLERANCE) || (fabsf(gyroZdiff) > GYRO_Z_TOLERANCE)){
		return('1');	//something gyroscopey is going on
	}
	
	return('0');	//nothing fishy is going on
}

void Setup_MPU6050(){
	simplePrint("Setup\n");
	//Sets sample rate to 1000/1+1 = 500Hz
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_SMPLRT_DIV, 0x01);
	//Disable FSync, 48Hz DLPF
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_CONFIG, 0x03);
	//Disable gyro self tests, scale of 500 degrees/s
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_CONFIG, 0b00001000);
	//Disable accel self tests, scale of +-4g, no DHPF
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_CONFIG, 0b00001000);
	//Freefall threshold of <|0mg|
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_FF_THR, 0x00);
	//Freefall duration limit of 0
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_FF_DUR, 0x00);
	//Motion threshold of >0mg
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_MOT_THR, 0x00);
	//Motion duration of >0s
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_MOT_DUR, 0x00);
	//Zero motion threshold
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_ZRMOT_THR, 0x00);
	//Zero motion duration threshold
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_ZRMOT_DUR, 0x00);
	//Disable sensor output to FIFO buffer
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_FIFO_EN, 0x00);

	//AUX I2C setup
	//Sets AUX I2C to single master control, plus other config
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_MST_CTRL, 0x00);
	//Setup AUX I2C slaves	
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV0_ADDR, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV0_REG, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV0_CTRL, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV1_ADDR, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV1_REG, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV1_CTRL, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV2_ADDR, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV2_REG, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV2_CTRL, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV3_ADDR, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV3_REG, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV3_CTRL, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_ADDR, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_REG, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_DO, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_CTRL, 0x00);
 	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_DI, 0x00);
 
 	//MPU6050_RA_I2C_MST_STATUS //Read-only
 	//Setup INT pin and AUX I2C pass through
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_INT_PIN_CFG, 0x00);
	//Enable data ready interrupt
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_INT_ENABLE, 0x00);

	//MPU6050_RA_DMP_INT_STATUS		//Read-only
	//MPU6050_RA_INT_STATUS 3A		//Read-only
    //MPU6050_RA_ACCEL_XOUT_H 		//Read-only
    //MPU6050_RA_ACCEL_XOUT_L 		//Read-only
    //MPU6050_RA_ACCEL_YOUT_H 		//Read-only
    //MPU6050_RA_ACCEL_YOUT_L 		//Read-only
    //MPU6050_RA_ACCEL_ZOUT_H 		//Read-only
    //MPU6050_RA_ACCEL_ZOUT_L 		//Read-only
    //MPU6050_RA_TEMP_OUT_H 		//Read-only
    //MPU6050_RA_TEMP_OUT_L 		//Read-only
    //MPU6050_RA_GYRO_XOUT_H 		//Read-only
    //MPU6050_RA_GYRO_XOUT_L 		//Read-only
    //MPU6050_RA_GYRO_YOUT_H 		//Read-only
    //MPU6050_RA_GYRO_YOUT_L 		//Read-only
    //MPU6050_RA_GYRO_ZOUT_H 		//Read-only
    //MPU6050_RA_GYRO_ZOUT_L 		//Read-only
    //MPU6050_RA_EXT_SENS_DATA_00 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_01 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_02 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_03 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_04 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_05 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_06 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_07 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_08 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_09 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_10 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_11 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_12 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_13 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_14 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_15 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_16 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_17 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_18 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_19 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_20 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_21 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_22 	//Read-only
    //MPU6050_RA_EXT_SENS_DATA_23 	//Read-only
    //MPU6050_RA_MOT_DETECT_STATUS 	//Read-only

	//Slave out, dont care
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV0_DO, 0x00);
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV1_DO, 0x00); 	
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV2_DO, 0x00); 	
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV3_DO, 0x00); 	
	//More slave config
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_MST_DELAY_CTRL, 0x00);
	//Reset sensor signal paths
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_SIGNAL_PATH_RESET, 0x00); 	
	//Motion detection control
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_MOT_DETECT_CTRL, 0x00); 		
	//Disables FIFO, AUX I2C, FIFO and I2C reset bits to 0
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_USER_CTRL, 0x00);
	//Sets clock source to gyro reference w/ PLL
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_1, 0b00000010);
	//Controls frequency of wakeups in accel low power mode plus the sensor standby modes
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_2, 0x00);
    //MPU6050_RA_BANK_SEL			//Not in datasheet
    //MPU6050_RA_MEM_START_ADDR		//Not in datasheet
    //MPU6050_RA_MEM_R_W			//Not in datasheet
    //MPU6050_RA_DMP_CFG_1			//Not in datasheet
    //MPU6050_RA_DMP_CFG_2			//Not in datasheet
    //MPU6050_RA_FIFO_COUNTH		//Read-only
    //MPU6050_RA_FIFO_COUNTL		//Read-only
	//Data transfer to and from the FIFO buffer
	LDByteWriteI2C(MPU6050_ADDRESS, MPU6050_RA_FIFO_R_W, 0x00);
    //MPU6050_RA_WHO_AM_I 			//Read-only, I2C address
		
}

int MPU6050_Test_I2C(){
	unsigned char Data = 0x00;
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_WHO_AM_I, &Data, 1);

	if(Data == 0x68){
		return(1);
		//printf("\nI2C Read Test Passed, MPU6050 Address: 0x%x", Data);
	}
	else{
		return(0);
		//printf("ERROR: I2C Read Test Failed, Stopping");	
	}	
}	
int MPU6050_Check_Registers(){
	unsigned char Data = 0x00;
	int Failed = 0;

	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_SMPLRT_DIV, &Data, 1);
	if(Data != 0x01) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_CONFIG, &Data, 1);
	if(Data != 0x03) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_CONFIG, &Data, 1);
	if(Data != 0b00001000) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_CONFIG, &Data, 1);
	if(Data != 0b00001000) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_FF_THR, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_FF_DUR, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_MOT_THR, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_MOT_DUR, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ZRMOT_THR, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ZRMOT_DUR, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_FIFO_EN, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_MST_CTRL, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV0_ADDR, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV0_REG, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV0_CTRL, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV1_ADDR, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV1_REG, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV1_CTRL, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV2_ADDR, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV2_REG, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV2_CTRL, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV3_ADDR, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV3_REG, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV3_CTRL, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_ADDR, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_REG, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_DO, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_CTRL, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
 	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV4_DI, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_INT_PIN_CFG, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_INT_ENABLE, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV0_DO, &Data, 1);
	if(Data != 0x00) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV1_DO, &Data, 1); 	
	if(Data != 0x00) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV2_DO, &Data, 1); 	
	if(Data != 0x00) {  Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_SLV3_DO, &Data, 1); 	
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_I2C_MST_DELAY_CTRL, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_SIGNAL_PATH_RESET, &Data, 1); 
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_MOT_DETECT_CTRL, &Data, 1); 	
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_USER_CTRL, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_1, &Data, 1);
	if(Data != 0x02) { ; Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_PWR_MGMT_2, &Data, 1);
	if(Data != 0x00) { Failed += 1; }
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_FIFO_R_W, &Data, 1);
	if(Data != 0x00) { Failed += 1; }

	//if (Failed == 0) { printf("\nRegister value check passed"); }
	//else { printf("\nRegister value check failed");}

	return(Failed);
}

void Calibrate_Gyros(){
	int x = 0;
	GYRO_XOUT_OFFSET_1000SUM = 0;
	GYRO_YOUT_OFFSET_1000SUM = 0;
	GYRO_ZOUT_OFFSET_1000SUM = 0;
	for(x = 0; x<5000; x++)
	{
		LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_XOUT_H, &GYRO_XOUT_H, 1);
		LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_XOUT_L, &GYRO_XOUT_L, 1);
		LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_YOUT_H, &GYRO_YOUT_H, 1);
		LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_YOUT_L, &GYRO_YOUT_L, 1);
		LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_ZOUT_H, &GYRO_ZOUT_H, 1);
		LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_ZOUT_L, &GYRO_ZOUT_L, 1);

		GYRO_XOUT_OFFSET_1000SUM += ((GYRO_XOUT_H<<8)|GYRO_XOUT_L);
		GYRO_YOUT_OFFSET_1000SUM += ((GYRO_YOUT_H<<8)|GYRO_YOUT_L);
		GYRO_ZOUT_OFFSET_1000SUM += ((GYRO_ZOUT_H<<8)|GYRO_ZOUT_L);

		delay(1);
	}	
	GYRO_XOUT_OFFSET = GYRO_XOUT_OFFSET_1000SUM/5000;
	GYRO_YOUT_OFFSET = GYRO_YOUT_OFFSET_1000SUM/5000;
	GYRO_ZOUT_OFFSET = GYRO_ZOUT_OFFSET_1000SUM/5000;
	
}	

//Gets raw accelerometer data, performs no processing
void Get_Accel_Values(){
	//simplePrint("Accel Values\n");
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, &ACCEL_XOUT_H, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_XOUT_L, &ACCEL_XOUT_L, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_YOUT_H, &ACCEL_YOUT_H, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_YOUT_L, &ACCEL_YOUT_L, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_ZOUT_H, &ACCEL_ZOUT_H, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_ACCEL_ZOUT_L, &ACCEL_ZOUT_L, 1);

	ACCEL_XOUT = ((ACCEL_XOUT_H<<8)|ACCEL_XOUT_L);
	ACCEL_YOUT = ((ACCEL_YOUT_H<<8)|ACCEL_YOUT_L);
	ACCEL_ZOUT = ((ACCEL_ZOUT_H<<8)|ACCEL_ZOUT_L);
}	

//Converts the already acquired accelerometer data into 3D euler angles
void Get_Accel_Angles(){
	//simplePrint("Accel Angles\n");
	//ACCEL_XANGLE = 57.295*atan((float)ACCEL_YOUT/ sqrt(pow((float)ACCEL_ZOUT,2)+pow((float)ACCEL_XOUT,2)))*a + (1-a)*ACCEL_XANGLE;
	//ACCEL_YANGLE = 57.295*atan((float)-ACCEL_XOUT/ sqrt(pow((float)ACCEL_ZOUT,2)+pow((float)ACCEL_YOUT,2)))*a + (1-a)*ACCEL_YANGLE;	

	ACCEL_XANGLE = 57.295*atan((float)ACCEL_YOUT/ sqrt(pow((float)ACCEL_ZOUT,2)+pow((float)ACCEL_XOUT,2)));
	ACCEL_YANGLE = 57.295*atan((float)-ACCEL_XOUT/ sqrt(pow((float)ACCEL_ZOUT,2)+pow((float)ACCEL_YOUT,2)));
}	

//Function to read the gyroscope rate data and convert it into degrees/s
void Get_Gyro_Rates(){
	//simplePrint("Gyro Rates\n");
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_XOUT_H, &GYRO_XOUT_H, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_XOUT_L, &GYRO_XOUT_L, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_YOUT_H, &GYRO_YOUT_H, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_YOUT_L, &GYRO_YOUT_L, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_ZOUT_H, &GYRO_ZOUT_H, 1);
	LDByteReadI2C(MPU6050_ADDRESS, MPU6050_RA_GYRO_ZOUT_L, &GYRO_ZOUT_L, 1);

	GYRO_XOUT = ((GYRO_XOUT_H<<8)|GYRO_XOUT_L) - GYRO_XOUT_OFFSET;
	GYRO_YOUT = ((GYRO_YOUT_H<<8)|GYRO_YOUT_L) - GYRO_YOUT_OFFSET;
	GYRO_ZOUT = ((GYRO_ZOUT_H<<8)|GYRO_ZOUT_L) - GYRO_ZOUT_OFFSET;


	GYRO_XRATE = (float)GYRO_XOUT/gyro_xsensitivity;
	GYRO_YRATE = (float)GYRO_YOUT/gyro_ysensitivity;
	GYRO_ZRATE = (float)GYRO_ZOUT/gyro_zsensitivity;

	GYRO_XANGLE += GYRO_XRATE*dt;
	GYRO_YANGLE += GYRO_YRATE*dt;
	GYRO_ZANGLE += GYRO_ZRATE*dt;
}	
