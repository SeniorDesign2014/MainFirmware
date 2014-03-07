#include "mk20dx128.h"
#include "core_pins.h"
#include "i2c.h"

/*********************************************************************
* Function:        InitI2C()
* Input:		None.
* Output:		None.
* Overview:		Initialises the I2C(1) peripheral
* Note:			Sets up Master mode, No slew rate control, 100Khz
********************************************************************/
void InitI2C(void){	
	//This function will initialize the I2C(1) peripheral.
	
	SIM_SCGC4 |= SIM_SCGC4_I2C0; //run I2C clock
	
	I2C0_A1 = 0;
	I2C0_RA = 0;
	I2C0_C2 |= I2C_C2_HDRS;
	
	CORE_PIN18_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
	CORE_PIN19_CONFIG = PORT_PCR_MUX(2)|PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
	
	I2C0_F = 0x27; // set frequency divider for 100k baud
	I2C0_FLT = 0x1F; 
	
	I2C0_C1 |= (I2C_C1_IICEN); //enable I2C
	
}

/*********************************************************************
* Function:        StartI2C()
* Input:		None.
* Output:		None.
* Overview:		Generates an I2C Start Condition
********************************************************************/
void StartI2C(void){
	//This function generates an I2C start condition 
	I2C0_C1 |= (I2C_C1_MST);	//Generate Start Condition
	//while (I2C_S_BUSY != (I2C0_S & I2C_S_BUSY));	//Wait for Start Condition (bus busy)
}

/*********************************************************************
* Function:        RestartI2C()
* Input:		None.
* Output:		None.
* Overview:		Generates a restart condition and optionally returns status
********************************************************************/
void RestartI2C(void){
	//This function generates an I2C Restart condition

	I2C0_C1 |= (I2C_C1_RSTA);		//Generate Restart		
	while (I2C_S_BUSY != (I2C0_S & I2C_S_BUSY));	//Wait for restart	(bus busy)
}

/*********************************************************************
* Function:        StopI2C()
* Input:		None.
* Output:		None.
* Overview:		Generates a bus stop condition
********************************************************************/
void StopI2C(void){
	//This function generates an I2C stop condition 

	I2C0_C1 &= !(I2C_C1_MST);		//Generate Stop Condition
	//while (I2C_S_BUSY == (I2C0_S & I2C_S_BUSY));	//Wait for Stop (bus idle)
}

/*********************************************************************
* Function:        WriteI2C()
* Input:		Byte to write.
* Output:		None.
* Overview:		Writes a byte out to the bus
********************************************************************/
void WriteI2C(unsigned char byte){
	//This function transmits the byte passed to the function
	I2C0_C1 |= I2C_C1_TX;			//enable transmit
	//IdleI2C();						//Wait for bus to be idle
	I2C0_D = byte;					//Load byte to I2C1 Transmit buffer
	//while (I2C_S_TCF != (I2C0_S & I2C_S_TCF));	//Wait for data transmission
}

/*********************************************************************
* Function:        IdleI2C()
* Input:		None.
* Output:		None.
* Overview:		Waits for bus to become Idle
********************************************************************/
void IdleI2C(void){
	while (I2C_S_BUSY == (I2C0_S & I2C_S_BUSY));		//Wait for bus Idle
}

/*********************************************************************
* Function:        ACKStatus()
* Input:		None.
* Output:		Acknowledge Status.
* Overview:		Return the Acknowledge status on the bus
********************************************************************/
unsigned int ACKStatus(void){
	return (I2C_S_RXAK == (I2C0_S & I2C_S_RXAK));		//Return Ack Status
}

/*********************************************************************
* Function:        NotAckI2C()
* Input:		None.
* Output:		None.
* Overview:		Generates a NO Acknowledge on the Bus
********************************************************************/
void NotAckI2C(void){
	I2C0_C1 &= !(I2C_C1_TXAK);		//Set for NotACk
}

/*********************************************************************
* Function:        AckI2C()
* Input:		None.
* Output:		None.
* Overview:		Generates an Acknowledge.
********************************************************************/
void AckI2C(void){
	I2C0_C1 |= (I2C_C1_TXAK);		//Set for ACk
}

/*********************************************************************
* Function:        getI2C()
* Input:		None.
* Output:		contents of I2C1 receive buffer.
* Overview:		Read a single byte from Bus
********************************************************************/
unsigned int getI2C(void){
	I2C0_C1 &= !(I2C_C1_TX);		//Enable Master receive
	return(I2C0_D);			//Return data in buffer
}

/*********************************************************************
* Function:       getsI2C()
* Input:		array pointer, Length.
* Output:		None.
* Overview:		read Length number of Bytes into array
********************************************************************/
unsigned int getsI2C(unsigned char *rdptr, unsigned char Length){
	while (Length --){
		*rdptr++ = getI2C();		//get a single byte

		if(Length == 1){
			NotAckI2C();		//Acknowledge all but last one
		}
	}
	AckI2C(); //turn Ack back on
	return(0);
}

/*********************************************************************
* Function:        LDByteWriteI2C()
* Input:		Control Byte, 8 - bit address, data.
* Output:		None.
* Overview:		Write a byte to low density device at address LowAdd
********************************************************************/
unsigned int LDByteWriteI2C(unsigned char ControlByte, unsigned char LowAdd, unsigned char data){
	unsigned int ErrorCode;

	//IdleI2C();						//Ensure Module is Idle
	StartI2C();						//Generate Start COndition
	WriteI2C(ControlByte);			//Write Control byte
	//IdleI2C();

	ErrorCode = ACKStatus();		//Return ACK Status
	
	WriteI2C(LowAdd);				//Write Low Address
	//IdleI2C();

	ErrorCode = ACKStatus();		//Return ACK Status

	WriteI2C(data);					//Write Data
	//IdleI2C();
	StopI2C();						//Initiate Stop Condition
	//EEAckPolling(ControlByte);		//Perform ACK polling
	return(ErrorCode);
}

/*********************************************************************
* Function:        LDByteReadI2C()
* Input:		Control Byte, Address, *Data, Length.
* Output:		None.
* Overview:		Performs a low density read of Length bytes and stores in *Data array
*				starting at Address.
********************************************************************/
void LDByteReadI2C(unsigned char ControlByte, unsigned char Address, unsigned char *Data, unsigned char Length){
	//IdleI2C();					//wait for bus Idle
	StartI2C();					//Generate Start Condition
	WriteI2C(ControlByte);		//Write Control Byte
	//IdleI2C();					//wait for bus Idle
	WriteI2C(Address);			//Write start address
	//IdleI2C();					//wait for bus Idle

	RestartI2C();				//Generate restart condition
	WriteI2C(ControlByte | 0x01);	//Write control byte for read
	//IdleI2C();					//wait for bus Idle
	
	getsI2C(Data, Length);		//read Length number of bytes
	StopI2C();					//Generate Stop
}

/*********************************************************************
* Function:        LDPageWriteI2C()
*
* Input:		ControlByte, LowAdd, *wrptr.
*
* Output:		None.
*
* Overview:		Write a page of data from array pointed to be wrptr
*				starting at LowAdd
*
* Note:			LowAdd must start on a page boundary
*******************************************************************
unsigned int LDPageWriteI2C(unsigned char ControlByte, unsigned char LowAdd, unsigned char *wrptr)
{
	IdleI2C();					//wait for bus Idle
	StartI2C();					//Generate Start condition
	WriteI2C(ControlByte);		//send controlbyte for a write
	IdleI2C();					//wait for bus Idle
	WriteI2C(LowAdd);			//send low address
	IdleI2C();					//wait for bus Idle
	putstringI2C(wrptr);		//send data
	IdleI2C();					//wait for bus Idle
	StopI2C();					//Generate Stop
	return(0);
}
*/

/*********************************************************************
* Function:        LDSequentialReadI2C()
* Input:		ControlByte, address, *rdptr, length.
* Output:		None.
* Overview:		Performs a sequential read of length bytes starting at address
*				and places data in array pointed to by *rdptr
*******************************************************************
unsigned int LDSequentialReadI2C(unsigned char ControlByte, unsigned char address, unsigned char *rdptr, unsigned char length){
	IdleI2C();						//Ensure Module is Idle
	StartI2C();						//Initiate start condition
	WriteI2C(ControlByte);			//write 1 byte
	IdleI2C();						//Ensure module is Idle
	WriteI2C(address);				//Write word address
	IdleI2C();						//Ensure module is idle
	RestartI2C();					//Generate I2C Restart Condition
	WriteI2C(ControlByte | 0x01);	//Write 1 byte - R/W bit should be 1 for read
	IdleI2C();						//Ensure bus is idle
	getsI2C(rdptr, length);			//Read in multiple bytes
	NotAckI2C();					//Send Not Ack
	StopI2C();						//Send stop condition
	return(0);
}
*/

/*********************************************************************
* Function:        EEAckPolling()
* Input:		Control byte.
* Output:		error state.
* Overview:		polls the bus for an Acknowledge from device
*******************************************************************
unsigned int EEAckPolling(unsigned char control){
	IdleI2C();				//wait for bus Idle
	StartI2C();				//Generate Start condition
	
	if(I2C1STATbits.BCL){
		return(-1);			//Bus collision, return
	}else{
		if(WriteI2C(control))
		{
			return(-3);		//error return
		}

		IdleI2C();			//wait for bus idle
		if(I2C1STATbits.BCL)
		{
			return(-1);		//error return
		}

		while(ACKStatus())
		{
			RestartI2C();	//generate restart
			if(I2C1STATbits.BCL)
			{
				return(-1);	//error return
			}

			if(WriteI2C(control))
			{
				return(-3);
			}

			IdleI2C();
		}
	}
	StopI2C();				//send stop condition
	if(I2C1STATbits.BCL){
		return(-1);
	}
	return(0);
}
*/

/*********************************************************************
* Function:        putstringI2C()
* Input:		pointer to array.
* Output:		None.
* Overview:		writes a string of data upto PAGESIZE from array
*******************************************************************
unsigned int putstringI2C(unsigned char *wrptr){
	unsigned char x;

	for(x = 0; x < PAGESIZE; x++)		//Transmit Data Until Pagesize
	{	
		if(WriteI2C(*wrptr))			//Write 1 byte
		{
			return(-3);				//Return with Write Collision
		}
		IdleI2C();					//Wait for Idle bus
		if(I2C1STATbits.ACKSTAT)
		{
			return(-2);				//Bus responded with Not ACK
		}
		wrptr++;
	}
	return(0);
}
*/
