//This file contains the function prototypes for the i2c function

//Low Level Functions
void InitI2C(void);
void IdleI2C(void);
void StartI2C(void);
void WriteI2C(unsigned char);
void StopI2C(void);
void RestartI2C(void);
unsigned int getsI2C(unsigned char*, unsigned char);
void NotAckI2C(void);
unsigned int ACKStatus(void);
unsigned int getI2C(void);
void AckI2C(void);
//unsigned int EEAckPolling(unsigned char);
//unsigned int putstringI2C(unsigned char*);

//High Level Functions for Low Density Devices
void LDByteReadI2C(unsigned char, unsigned char, unsigned char*, unsigned char);
unsigned int LDByteWriteI2C(unsigned char, unsigned char, unsigned char);
//unsigned int LDPageWriteI2C(unsigned char, unsigned char, unsigned char*);
//unsigned int LDSequentialReadI2C(unsigned char, unsigned char, unsigned char*, unsigned char);