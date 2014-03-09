/*******************************************************************************
 *  rtc.c
 *  Teensy3
 *
 * Purpose:     Provides routines for RTC alarm.
 *
 *******************************************************************************/

#include "rtc.h"

void rtc_alarm(unsigned long sec) {
    if (!(RTC_IER & RTC_IER_TAIE_MASK)) RTC_IER |= RTC_IER_TAIE_MASK;
    RTC_TAR = rtc_get() + (sec - 1);
}

void rtc_stop(void) {
    RTC_IER &= ~RTC_IER_TAIE_MASK;
}