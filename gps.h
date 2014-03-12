#ifndef GPS_H
#define GPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mk20dx128.h"
#include "core_pins.h"
#include <stdio.h>
#include <string.h>
#include "HardwareSerial.h"

/* Needs RX_BUFFER_SIZE to be set to 96 */

/*Nick makes a struct!*/
struct location{
	char lat[4];
	char lon[5];
	char lat_min[9];
	char lon_min[9];
	char vel[6];
	//char deg[5];
};


void gps_init(void);
int gps_parse(struct location* data);
void gps_pwrup(void);
void gps_pwrdwn(void);
void gps_sleep(void);
void gps_wake(void);
void gps_end(void);
void gps_pack_message(char* message, struct location* data, char stolen);
#ifdef __cplusplus
}
#endif

#endif
