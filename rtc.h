/*
 *  rtc.h
 *  Teensy3
 *
 */

#ifndef __RTC_H__
#define __RTC_H__
/********************************************************************/
#include "mk20dx128.h"
#include "mk20dx128_ext.h"
//#include "Arduino.h"
/********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
    // function prototypes
    void rtc_alarm(unsigned long sec);
    void rtc_stop(void);
#ifdef __cplusplus
}
#endif
/********************************************************************/
#endif /* __RTC_H__ */
