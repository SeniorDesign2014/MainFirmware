/*
 ||
 || @file 		LowPower_Teensy3.h
 || @version 	4
 || @author 	Colin Duffy
 || @contact 	cmduffy@engr.psu.edu
 ||
 || @description
 || # Low Power Library for Teensy 3.0/3.1.
 ||
 || @license
 || | Copyright (c) 2014 Colin Duffy
 || | This library is free software; you can redistribute it and/or
 || | modify it under the terms of the GNU Lesser General Public
 || | License as published by the Free Software Foundation; version
 || | 2.1 of the License.
 || |
 || | This library is distributed in the hope that it will be useful,
 || | but WITHOUT ANY WARRANTY; without even the implied warranty of
 || | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 || | Lesser General Public License for more details.
 || |
 || | You should have received a copy of the GNU Lesser General Public
 || | License along with this library; if not, write to the Free Software
 || | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 || #
 ||
 */

#ifndef LowPower_Teensy3
#define LowPower_Teensy3
#include "Arduino.h"
#include "utility/mk20dx128_ext.h"
#include "utility/t3core2lp.h"
#include "utility/module.h"

/* Define LLS & VLLS Wakeup Pin */
#define PIN_2          0x1000
#define PIN_4          0x10
#define PIN_6          0x4000
#define PIN_7          0x2000
#define PIN_9          0x80
#define PIN_10         0x100
#define PIN_11         0x200
#define PIN_13         0x400
#define PIN_16         0x20
#define PIN_21         0x8000
#define PIN_22         0x40
#define PIN_26         0x01
#define PIN_30         0x800
#define PIN_33         0x02

/* Define Pin Interrupt Type */
#define PIN_DISABLED    0x00
#define PIN_RISING      0x01
#define PIN_FALLING     0x02
#define PIN_ANY         0x03

/* Define Module Wakeup Source */
#define GPIO_WAKE       0x01
#define LPTMR_WAKE      0x10000
#define RTCA_WAKE       0x200000
#define RTCS_WAKE       0x400000
#define CMP0_WAKE       0x20000
#define CMP1_WAKE       0x40000
#define TSI_WAKE        0x100000

/* Hardware Serial Baud VLPR Mode */
#define TWO_MHZ     2000000
#define FOUR_MHZ    4000000
#define EIGHT_MHZ   8000000
#define SIXTEEN_MHZ 16000000

#define LP_BAUD2DIV(baud, cpu) (((cpu * 2) + ((baud) >> 1)) / (baud))
#define LP_BAUD2DIV3(baud, bus) (((bus * 2) + ((baud) >> 1)) / (baud))

class TEENSY3_LP;

struct configSleep {
    /* Module Wake Type */
    uint32_t modules;
    /* Structure GPIO Config */
    uint16_t gpio_pin;
    uint16_t gpio_mode;
    /* Structure LPTMR Config */
    uint16_t lptmr_timeout;
    /* Structure RTC Config */
    unsigned long rtc_alarm;
    /* Structure TSI Config */
    uint16_t tsi_threshold;
    uint8_t tsi_pin;
    /* Structure wake source */
    uint32_t wake_source;
    /* pointer to callback function */
    void (*callback)();
};

class TEENSY3_LP {
private:
    /* Handler Functions */
    void gpioHandle(uint32_t pin, uint8_t pinType);
    void lptmrHandle(uint32_t timeout);
    void rtcHandle(unsigned long unixSec);
    void cmpHandle(void);
    void tsiHandle(uint8_t var, uint16_t threshold);
    inline bool sleepHandle(const char* caller, uint32_t wakeType, uint32_t var1, uint16_t var2) __attribute__((always_inline)) ;
    void sleepHandle(volatile struct configSleep* config);
    /* TSI Initialize  */
    void tsiIntialize(void);
    /* private class access to wakeup ISR  */
    friend void wakeup_isr(void);
    /* handle call to user callback  */
    typedef void (*ISR)();
    static ISR CALLBACK;
    /* default callback ISR  */
    static void defaultCallback() { yield(); };
    
    static volatile uint32_t wakeSource;// hold llwu wake up source for wakeup isr
    static volatile uint32_t stopflag;// hold module wake up sources for wakeup isr
    static volatile uint8_t lowLeakageSource;// hold lowleakage mode for wakeup isr
    
    //friend class IntervalTimer_LP;
    friend class HardwareSerial_LP;
    friend class HardwareSerial2_LP;
    friend class HardwareSerial3_LP;
    friend class IntervalTimer_LP;
    static volatile uint32_t _cpu;
    static volatile uint32_t _bus;
    static volatile uint32_t _mem;
public:
    // Constructor
    TEENSY3_LP(void);
    //---------------------------------------------------------------------------------------
    /* Sleep Functions */
    //----------------------------------------CPU--------------------------------------------
    int CPU(uint32_t freq);
    //---------------------------------------Sleep-------------------------------------------
    void Sleep();
    //--------------------------------------DeepSleep----------------------------------------
    void DeepSleep(uint32_t wakeType, uint32_t var1, uint16_t var2, ISR myCallback);
    void DeepSleep(uint32_t wakeType, uint32_t var1, ISR myCallback) { DeepSleep(wakeType, var1, 0, myCallback); }
    void DeepSleep(uint32_t wakeType, uint32_t var1, uint16_t var2) { DeepSleep(wakeType, var1, var2, defaultCallback); }
    void DeepSleep(uint32_t wakeType, uint32_t var1) { DeepSleep(wakeType, var1, 0, defaultCallback); }
    void DeepSleep(volatile struct configSleep* config);
    //--------------------------------------Hibernate----------------------------------------
    void Hibernate(uint32_t wakeType, uint32_t var1, uint16_t var2, ISR myCallback);
    void Hibernate(uint32_t wakeType, uint32_t var1, uint16_t var2) { Hibernate(wakeType, var1, var2, defaultCallback); }
    void Hibernate(uint32_t wakeType, uint32_t var1, ISR myCallback) { Hibernate(wakeType, var1, 0, myCallback); }
    void Hibernate(uint32_t wakeType, uint32_t var1) { Hibernate(wakeType, var1, 0, defaultCallback); }
    void Hibernate(volatile struct configSleep* config);
    //---------------------------------------PrintSRS----------------------------------------
    void PrintSRS(Stream *port);
    //-----------------------------------------Core------------------------------------------
    uint32_t cpuFreq(void) { return _cpu; }
    static uint32_t micros() { micros_lp(_cpu); }
    static void delay(uint32_t msec) { delay_lp(msec,_cpu); }
    static void delayMicroseconds(uint32_t usec) { delayMicroseconds_lp(usec, _cpu); }
    
};

/**** !!!!!Must make interval timer private members protected for this to work!!!! *****/
class IntervalTimer_LP : public IntervalTimer {
private:
public:
    bool begin(ISR newISR, unsigned int newPeriod) {
        if (newPeriod == 0 || newPeriod > MAX_PERIOD) return false;
        uint32_t newValue = (TEENSY3_LP::_cpu / 1000000) * newPeriod - 1;
        return beginCycles(newISR, newValue);
    }
};

class HardwareSerial_LP : public HardwareSerial {
private:
public:
    void begin(uint32_t baud) {
        serial_begin(LP_BAUD2DIV(baud, TEENSY3_LP::_cpu));
    }
    void begin(uint32_t baud, uint32_t format) {
        serial_begin(LP_BAUD2DIV(baud, TEENSY3_LP::_cpu));
        serial_format(format);
    }
    void end(void) {
        serial_end();
        uart1Disable();
    }
};

class HardwareSerial2_LP : public HardwareSerial2 {
private:
public:
    void begin(uint32_t baud) { serial2_begin(LP_BAUD2DIV(baud, TEENSY3_LP::_cpu)); }
    void begin(uint32_t baud, uint32_t format) {
        serial2_begin(LP_BAUD2DIV(baud, TEENSY3_LP::_cpu));
        serial2_format(format);
    }
    void end(void) {
        serial2_end();
        uart2Disable();
    }
};

class HardwareSerial3_LP : public HardwareSerial3 {
private:
public:
    void begin(uint32_t baud) { serial3_begin(LP_BAUD2DIV3(baud, TEENSY3_LP::_bus)); }
    void begin(uint32_t baud, uint32_t format) {
        serial3_begin(LP_BAUD2DIV3(baud, TEENSY3_LP::_bus));
        serial3_format(format);
    }
    void end(void) {
        serial3_end();
        uart3Disable();
    }
};

#endif