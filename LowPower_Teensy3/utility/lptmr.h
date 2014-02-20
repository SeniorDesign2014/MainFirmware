/*
 *  lptm.h
 *  Teensy3
 *
 */
#ifndef __LPTMR_H__
#define __LPTMR_H__
/********************************************************************/
#include <inttypes.h>
#include "mk20dx128.h"
#include "mk20dx128_ext.h"

/********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
    // function prototypes
    /* Low Power Timer LPTM Functions */
    void lptmr_start(float period);// Start the LPTMR Timer in msec
    void lptmr_stop(void);// Stop LPTM timer
#ifdef __cplusplus
}
#endif
/********************************************************************/
#endif /* defined(__LPTMR_H__) */
