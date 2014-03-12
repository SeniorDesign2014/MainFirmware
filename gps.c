#include "gps.h"


/*Checking incoming messages for NEMEA GPGLL and GPVTG messages.
Returns with negative values if messages contain invalid data,
returns positive values for valid data, and zero for messages
it doesn't recognize*/
int gps_parse(struct location* data){
	char rx_buf[128];
	int  buf = 0;
	int i = 0;	

	buf = serial2_getchar();

	if(buf == 0x24){
		while(rx_buf[i-1] != 0x0A){
			buf = serial2_getchar();
			//usb_serial_putchar(buf);
			if(buf != -1){
				rx_buf[i] = buf;
				i++;
			}
		}
	}
	
	if(strncmp(rx_buf, "GPGLL", 5) == 0){
		if(rx_buf[6] == ','){
			//invalid data
			return(-1);
		}
		
		//latitude
		if(rx_buf[17] == 'S'){
			data->lat[0] = '-';
		}else{
			data->lat[0] = '0';
		}
		for(i=1; i<3; i++){
			data->lat[i] = rx_buf[5+i];
		}
		data->lat[3] = '\0';

		for(i=0; i<8; i++){
			data->lat_min[i] = rx_buf[8+i];
		}
		data->lat_min[8] = '\0';
			
		//longitude 
		if(rx_buf[31] == 'W'){
			data->lon[0] = '-';
		}else{
			data->lon[0] = '0';
		}
		for(i=1; i<4; i++){
			data->lon[i] = rx_buf[18+i];
		}
		data->lon[4] = '\0';
		
		for(i=0; i<8; i++){
			data->lon_min[i] = rx_buf[22+i];
		}
		data->lon_min[8] = '\0';
		return(1);
	}

	if(strncmp(rx_buf, "GPVTG", 5) == 0){
		if(rx_buf[12] == ','){
			//invalid data
			return(-2);
			}
		for(i=0; i< 5; i++){
			data->vel[i] = rx_buf[20+i];
		}
		data->vel[5] = '\0';
		return(2);
	}

	return(0);
}

/* Turns off extra messages from GPS.
TODO: Shut off timepulse.
*/
void gps_init(void){
	char cfg_tp5[] = { 0xB5, 0x62, 0x06, 0x31, 0x00, 0x00, 0x00, 0x00};

	//Initialize Serial2 Port
	serial2_begin(BAUD2DIV(9600));
	serial2_format(SERIAL_8N1);

	//Powers up GPS module if it was told to turn off
	gps_pwrup();

	//Quiet extra message
	serial2_write("$PUBX,40,GGA,0,0,0,0*5A\r\n", 25);
	serial2_write("$PUBX,40,GSA,0,0,0,0*4E\r\n", 25);
	serial2_write("$PUBX,40,GSV,0,0,0,0*59\r\n", 25);
	serial2_write("$PUBX,40,RMC,0,0,0,0*47\r\n", 25);

	serial2_write(cfg_tp5, sizeof(cfg_tp5));


	
}
//Deep sleep. Powers off entire module.
void gps_pwrdwn(void){
	char pwr_dwn[] = { 0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B};

	serial2_write(pwr_dwn, sizeof(pwr_dwn));
}

//Deep wake up. Wake up from power off
void gps_pwrup(void){
	char pwr_up[] = { 0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x4C, 0x37};
	
	serial2_write(pwr_up, sizeof(pwr_up));
}

//Normal sleep. Turns off RF portion only. Keeps settings.
void gps_sleep(void){
	char sleep[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00,0x08, 0x00, 0x16, 0x74};
	
	serial2_write(sleep, sizeof(sleep));
}


//Normal wake. Turns on RF portion.
void gps_wake(void){
	char wake_up[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00,0x09, 0x00, 0x17, 0x76};
	
	serial2_write(wake_up, sizeof(wake_up));
}

void gps_end(void){
	gps_pwrdwn();
	serial2_end();
}

void gps_pack_message(char* message, struct location* data, char stolen){
	sprintf(message, "\"clientid\":\"00000001\",\"x\":\"%s\",\"xm\":\"%s\",\"y\":\"%s\",\"ym\":\"%s\",\"vel\":\"%s\",\"stolen\":\"%c\"", data->lat, data->lat_min, data->lon, data->lon_min, data->vel, stolen); 
}
