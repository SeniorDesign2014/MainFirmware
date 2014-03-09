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
	char lat[3];
	char lon[4];
	char lat_min[7];
	char lon_min[7];
	char vel[5];
	//char deg[5];
};


void gps_init(void);
int gps_parse(struct location* data);
void gps_pwrup(void);
void gps_pwrdwn(void);
void gps_sleep(void);
void gps_wake(void);
void gps_end(void);

#ifdef __cplusplus
}
#endif

#endif
