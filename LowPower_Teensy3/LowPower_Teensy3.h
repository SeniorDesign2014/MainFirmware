/*
 *  LowPower_Teensy3.h
 *  Teensy3
 *
 *  Created by colin duffy on 4/26/13.
 */

#ifndef LowPower_Teensy3
#define LowPower_Teensy3

#include "utility/mk20dx128_ext.h"
#include "utility/lptmr.h"
#include "utility/llwu.h"
#include "utility/smc.h"
#include "utility/mcg.h"
#include "utility/rtc.h"
#include "utility/tsi.h"

/* Define Wakeup Pin */
#define PIN_2           0x1000
#define PIN_4           0x10
#define PIN_6           0x4000
#define PIN_7           0x2000
#define PIN_9           0x80
#define PIN_10          0x100
#define PIN_11          0x400
#define PIN_16          0x20
#define PIN_21          0x8000
#define PIN_22          0x40

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

/* Define Low Power Run ON/OFF */
#define LP_RUN_ON       0x01
#define LP_RUN_OFF      0x02

#define NO_WAKE_ON_INTERRUPT  0x00
#define WAKE_ON_INTERRUPT     0x01

/* Define Low Leakage Source */
#define LLS             0x01
#define VLLS            0x02

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
};    
    
class TEENSY3_LP {
private:
    /* Sleep Functions */
    void gpioHandle(uint32_t pin, uint8_t pinType);
    void lptmrHandle(float timeout);
    void rtcHandle(unsigned long unixSec);
    void cmpHandle(void);
    void tsiHandle(uint8_t var, uint16_t threshold);
    /* TSI Intialize  */
    void tsiIntialize(void);
public:
    TEENSY3_LP();// Constructor
    /* Sleep Functions */
    void PrintSRS(void);
    void Run(uint8_t mode);
    void Run(uint8_t mode, uint8_t woi);
    void Wait(void);
    void Sleep(void);
    void DeepSleep(uint32_t wakeType, uint32_t var1, uint16_t var2);
    void DeepSleep(uint32_t wakeType, uint32_t var);
    void DeepSleep(volatile struct configSleep* config);
    void Hibernate(uint32_t wakeType, uint32_t var1, uint16_t var2);
    void Hibernate(uint32_t wakeType, uint32_t var);
    void Hibernate(volatile struct configSleep* config);
};

#endif