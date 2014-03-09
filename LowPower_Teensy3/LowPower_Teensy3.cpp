/*******************************************************************************
 *  LowPower_Teensy3.cpp
 *  Teensy3
 *
 * Purpose:    Provides routines for configuring the Teensy3 for low power.
 *
 * NOTE:       None
 *******************************************************************************/

#include "LowPower_Teensy3.h"
#include "utility/lptmr.h"
#include "utility/llwu.h"
#include "utility/smc.h"
#include "utility/mcg.h"
#include "utility/rtc.h"
#include "utility/tsi.h"
#include "utility/cmp.h"
#include "utility/module.h"
#include "util/atomic.h"
#include "utility/bitband.h"

/* Define Low Leakage Source */
#define LLS             0x01
#define VLLS            0x02

/* CPU Freq in VLPR mode */
#define BLPI_CPU    2000000
#define BLPI_BUS    2000000
#define BLPI_MEM    1000000

#define BLPE_CPU    4000000
#define BLPE_BUS    4000000
#define BLPE_MEM    1000000

volatile uint32_t TEENSY3_LP::wakeSource;// hold llwu wake up source for wakeup isr
volatile uint32_t TEENSY3_LP::stopflag;// hold module wake up sources for wakeup isr
volatile uint8_t TEENSY3_LP::lowLeakageSource;// hold lowleakage mode for wakeup isr
volatile uint32_t TEENSY3_LP::_cpu;
volatile uint32_t TEENSY3_LP::_bus;
volatile uint32_t TEENSY3_LP::_mem;

TEENSY3_LP::TEENSY3_LP() {
    //assign callback to default callback
    //CALLBACK = defaultCallback;
    // Enable all wakeup types - SMC_PMPROT: AVLP=1,ALLS=1,AVLLS=1
    SMC_PMPROT = SMC_PMPROT_ALLS_MASK | SMC_PMPROT_AVLLS_MASK | SMC_PMPROT_AVLP_MASK;
    /*
     This is an advanced feature, usb standby during low power modes
     is disabled by default. The regulator can only supply a limited
     amount of current if you choose use this feature, use with caution!
     */
    // allows write to SIM_SOPT1CFG USBSSTBY register
    SIM_SOPT1CFG |= SIM_SOPT1CFG_USSWE_MASK;
    // clear usb regulator standby bit in LLS & VLLS modes, no standby
    SIM_SOPT1 &= ~SIM_SOPT1_USBSSTBY_MASK;
    
    // allows write to SIM_SOPT1CFG USBVSTBY register
    SIM_SOPT1CFG |= SIM_SOPT1CFG_UVSWE_MASK;
    // clear usb regulator standby bit in VLPR
    SIM_SOPT1 &= ~SIM_SOPT1_USBVSTBY_MASK;
    // clear llwu flags
    wakeSource = llwu_clear_flags();
    // initialize
    _cpu = F_CPU;
    _bus = F_BUS;
    _mem = F_MEM;
}
/****************************** Func *******************************
 * Routines to enable different sleep modes on the teensy3.
 *
 **void wake_isr() - 
 *      handles wake ups from LLS or VLLS. It clears LLWU flags
 *      sets MCG_C1 to PEE run mode clears flags and sets wakeup
 *      variable.
 * Arguments: NONE
 *
 **void defaultCallback() -
 *      gets called if no user callback function is defined.
 * Arguments: NONE
 *
 **void CPU(uint32_t cpu) -
 *      dynamically configures the core cpu(2, 4, 8, 16, 24, 48, 96 MHZ),
 *      bus and flash clocks and also configures the SysTick for the
 *      selected freq.
 * Arguments:  cpu -> TWO_MHZ, FOUR_MHZ, EIGHT_MHZ, SIXTEEN_MHZ, F_CPU
 * 
 **void Sleep() - 
 *      Very versatile sleep option that the can be woken up by many more sources than 
 *      other modes. This puts the processor into Wait Mode and disables the systick, 
 *      any digital pin using attachInterrupt or any of the timers can wake it from 
 *      this Sleep Mode.
 * Arguments: NONE //TODO: add option to disable modules
 * 
 **void DeepSleep(uint8_t wakeType, uint32_t var, uint16_t var2, void (*callbackfunc)()) - 
 *      set teensy3 to LLS sleep mode. Exited by Low-Power Timer, 
 *      Digital Pin, RTC and TSI; Code execution begins after call 
 *      to DeepSleep function. User callback function will proceed from wakeup_isr()
 * Arguments:  wakeType -> LPTMR_WAKE, GPIO_WAKE, RTCA_WAKE, TSI_WAKE defines
 *              var -> LPTMR_WAKE = timeout in msec
 *              var -> GPIO_WAKE = wake pin, which are defined in the header
 *              var -> RTCA_WAKE = alarm in secs
 *              var -> TSI_WAKE = wake pin, only one pin can selected
 *              var2 -> TSI wakeup threshold
 *              void (*callbackfunc)() -> pointer to user callback function from wakeup_isr()
 *
 **void DeepSleep(volatile struct configSleep* config) -
 *      uses the struct to configure the teensy wake sources. This
 *      allows the teensy to have multiple wakeup sources and
 *      multiple configurations.
 * Arguments:  configSleep* struct - defined in header
 *
 *
 **void Hibernate(uint8_t wakeType, uint32_t var, uint16_t var2, void (*callbackfunc)()) - 
 *      set teensy3 to VLLS3 sleep mode. Exited by Low-Power Timer,
 *      Digital Pin, RTC and TSI; Code execution begins after call
 *      to DeepSleep function. Code execution begins through reset flow.
 * Arguments:  wakeType -> LPTMR_WAKE, GPIO_WAKE, RTCA_WAKE, TSI_WAKE defines
 *              var -> LPTMR_WAKE = timeout in msec
 *              var -> GPIO_WAKE = wake pin, which are defined in the header
 *              var -> RTCA_WAKE = alarm in secs
 *              var -> TSI_WAKE = wake pin, only one pin can selected
 *              var2 -> TSI threshold value
 *              void (*callbackfunc)() -> pointer to user callback function from wakeup_isr()
 *
 **void Hibernate(volatile struct configSleep* config) -
 *      uses the struct to configure the teensy wake sources. This
 *      allows the teensy to have multiple wakeup sources and
 *      multiple configurations.
 * Arguments:  configSleep* struct - defined in header
 *
 *
 **void PrintSRS() - 
 *      prints the reset type and current power mode.
 ***********************************************************************/

/******************************** ISR: *********************************/
TEENSY3_LP::ISR TEENSY3_LP::CALLBACK;

void wakeup_isr(void) {
    
    NVIC_DISABLE_IRQ(IRQ_LLWU); // disable wakeup isr
    
    uint32_t llwuFlag;
    
    llwuFlag = llwu_clear_flags();// clear llwu flags after wakeup a/ store wakeup source
    
    pbe_pee();// mcu is in PBE from LLS wakeup, transition back to PEE (if exiting from normal RUN mode)

    // clear wakeup module and stop them
    if ((TEENSY3_LP::stopflag & LPTMR_WAKE) && (TEENSY3_LP::lowLeakageSource == LLS)) lptmr_stop();
    if ((TEENSY3_LP::stopflag & RTCA_WAKE) && (TEENSY3_LP::lowLeakageSource == LLS)) rtc_stop();
    if ((TEENSY3_LP::stopflag & TSI_WAKE) && (TEENSY3_LP::lowLeakageSource == LLS)) tsi_stop();
    if ((TEENSY3_LP::stopflag & CMP0_WAKE) && (TEENSY3_LP::lowLeakageSource == LLS)) cmp_stop();
    
    TEENSY3_LP::CALLBACK();
    
    TEENSY3_LP *p;
    
    p->wakeSource = llwuFlag;
}
/***************************** PUBLIC: ******************************/
int TEENSY3_LP::CPU(uint32_t cpu) {
    if (_cpu == cpu) return 0;
    /********************************************/
    /* First check if we are in blpi or blpe, if
    /* so transition to pee at F_CPU, F_BUS, F_MEM.
    /********************************************/
    if (mcg_mode() == BLPI) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            // exit low power Run
            exit_vlpr();
            blpi_pee();
        }
        usbEnable();
    } else if (mcg_mode() == BLPE) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            // exit low power Run
            if (SMC_PMSTAT == 0x04) exit_vlpr();
            blpe_pee();
            usbEnable();
        }
    }
    if (cpu >= 24000000) {
        // config divisors: F_CPU core, F_BUS bus, F_MEM flash
        //ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            _cpu = F_CPU;
            _bus = F_BUS;
            _mem = F_MEM;
        //}
        return F_CPU;
    } else if (cpu <= FOUR_MHZ) {
        if (cpu == TWO_MHZ) {
            _cpu = BLPI_CPU;
            _bus = BLPI_BUS;
            _mem = BLPI_MEM;
            usbDisable();
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                // transition from PEE to BLPI
                pee_blpi();
                // config divisors: 2 MHz core, 2 MHz bus, 1 MHz flash
                mcg_cpu(0x00, 0x00, 0x01, 1999);
                // now safe to enter vlpr
                enter_vlpr(0);
                systick_millis_count = 0;
                SYST_CVR = 0;
            }
            return TWO_MHZ;
        } else if (cpu == FOUR_MHZ) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                _cpu = BLPE_CPU;
                _bus = BLPE_BUS;
                _mem = BLPE_MEM;
                usbDisable();
                // transition from PEE to BLPE
                pee_blpe();
                // config divisors: 4 MHz core, 4 MHz bus, 1 MHz flash
                mcg_cpu(0x03, 0x03, 0x0F, 3999);
                // now safe to enter vlpr
                enter_vlpr(0);
                systick_millis_count = 0;
                SYST_CVR = 0;
            }
            return FOUR_MHZ;
        } else return -1;
    } else if (cpu == EIGHT_MHZ) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            _cpu = EIGHT_MHZ;
            _bus = EIGHT_MHZ;
            _mem = EIGHT_MHZ;
            usbDisable();
            // transition from PEE to BLPE
            pee_blpe();
            // config divisors: 8 MHz core, 8 MHz bus, 8 MHz flash
            mcg_cpu(0x01, 0x01, 0x01, 7999);
            systick_millis_count = 0;
            SYST_CVR = 0;
        }
        return EIGHT_MHZ;
    } else if (cpu == SIXTEEN_MHZ) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            _cpu = SIXTEEN_MHZ;
            _bus = SIXTEEN_MHZ;
            _mem = SIXTEEN_MHZ;
            usbDisable();
            // transition from PEE to BLPE
            pee_blpe();
            // config divisors: 16 MHz core, 16 MHz bus, 16 MHz flash
            mcg_cpu(0x00, 0x00, 0x00, 15999);
            systick_millis_count = 0;
            SYST_CVR = 0;
        }
        return SIXTEEN_MHZ;
    } else {
        _cpu = F_CPU;
        _bus = F_BUS;
        _mem = F_MEM;
        return -1;
    }
}
//----------------------------------------------------------------------------------------------------------
void TEENSY3_LP::Sleep() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        usbDisable();
        //vrefDisable();
        //adcDisable();
        //rtcDisable();
        CPU(TWO_MHZ);
        enter_wait();
        CPU(F_CPU);
        //rtcEnable();
        //adcEnable();
        //vrefEnable();
        usbEnable();
    }
}
//----------------------------------------------------------------------------------------------------------
void TEENSY3_LP::DeepSleep(uint32_t wakeType, uint32_t var1, uint16_t var2, ISR callback) {
    if (callback == NULL) {
        CALLBACK = defaultCallback;
    } else CALLBACK = callback;
    
    stopflag = 0;

    sleepHandle(__FUNCTION__, wakeType, var1, var2);
}

void TEENSY3_LP::DeepSleep(volatile struct configSleep* config) {
    if (config->callback == NULL) {
    	CALLBACK = defaultCallback;
    } else {
       	CALLBACK = config->callback;
    }
   
    lowLeakageSource = LLS;
    
    stopflag = 0;
    
    if (config->modules & GPIO_WAKE) {
        gpioHandle(config->gpio_pin, PIN_ANY);
    }
    if (config->modules & LPTMR_WAKE) {
        stopflag |= LPTMR_WAKE;
        lptmrHandle(config->lptmr_timeout);
    }
    if (config->modules & RTCA_WAKE) {
        stopflag |= RTCA_WAKE;//0x02;
        rtcHandle(config->rtc_alarm);
    }
    if (config->modules & RTCS_WAKE) {
        stopflag |= RTCS_WAKE;
    }
    if (config->modules & CMP0_WAKE) {
        stopflag |= CMP0_WAKE;
    }
    if (config->modules & CMP1_WAKE) {
        stopflag |= CMP1_WAKE;
    }
    if (config->modules & TSI_WAKE) {
        stopflag |= TSI_WAKE;
        tsiHandle(config->tsi_pin, config->tsi_threshold);
    }
    
    llwu_configure(config->gpio_pin, PIN_ANY, config->modules);// configure llwu
    
    NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
    
    enter_lls();// enter lls sleep mode
    
    config->wake_source = wakeSource;// who woke me up?
    
}
//----------------------------------------------------------------------------------------------------------
void TEENSY3_LP::Hibernate(uint32_t wakeType, uint32_t var1, uint16_t var2, ISR callback) {
    if (callback == NULL) {
        CALLBACK = defaultCallback;
    } else CALLBACK = callback;
    
    CALLBACK = callback;
    
    sleepHandle(__FUNCTION__, wakeType, var1, var2);
}

void TEENSY3_LP::Hibernate(volatile struct configSleep* config) {
    if (config->callback == NULL) {
        CALLBACK = defaultCallback;
    } else {
        CALLBACK = config->callback;
    }
    
    lowLeakageSource = VLLS;
    
    stopflag = 0;
    
    if (config->modules & GPIO_WAKE) {
        gpioHandle(config->gpio_pin, PIN_ANY);
    }
    if (config->modules & LPTMR_WAKE) {
        stopflag |= LPTMR_WAKE;
        lptmrHandle(config->lptmr_timeout);
    }
    if (config->modules & RTCA_WAKE) {
        stopflag |= RTCA_WAKE;
        rtcHandle(config->rtc_alarm);
    }
    if (config->modules & RTCS_WAKE) {
        stopflag |= RTCS_WAKE;
    }
    if (config->modules & CMP0_WAKE) {
        stopflag |= CMP0_WAKE;
    }
    if (config->modules & CMP1_WAKE) {
        stopflag |= CMP1_WAKE;
    }
    if (config->modules & TSI_WAKE) {
        stopflag |= TSI_WAKE;
        tsiHandle(config->tsi_pin, config->tsi_threshold);
    }

    llwu_configure(config->gpio_pin, PIN_ANY, config->modules);
    
    NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
    
    enter_vlls3();// enter vlls3 sleep mode*/
}
//----------------------------------------------------------------------------------------------------------
void TEENSY3_LP::PrintSRS(Stream *port) {
    port->println("------------------------------------------");
    if (RCM_SRS1 & RCM_SRS1_SACKERR_MASK) port->println("[RCM_SRS0] - Stop Mode Acknowledge Error Reset");
    if (RCM_SRS1 & RCM_SRS1_MDM_AP_MASK) port->println("[RCM_SRS0] - MDM-AP Reset");
    if (RCM_SRS1 & RCM_SRS1_SW_MASK) port->println("[RCM_SRS0] - Software Reset");
    if (RCM_SRS1 & RCM_SRS1_LOCKUP_MASK) port->println("[RCM_SRS0] - Core Lockup Event Reset");
    if (RCM_SRS0 & RCM_SRS0_POR_MASK) port->println("[RCM_SRS0] - Power-on Reset");
    if (RCM_SRS0 & RCM_SRS0_PIN_MASK) port->println("[RCM_SRS0] - External Pin Reset");
    if (RCM_SRS0 & RCM_SRS0_WDOG_MASK) port->println("[RCM_SRS0] - Watchdog(COP) Reset");
    if (RCM_SRS0 & RCM_SRS0_LOC_MASK) port->println("[RCM_SRS0] - Loss of External Clock Reset");
    if (RCM_SRS0 & RCM_SRS0_LOL_MASK) port->println("[RCM_SRS0] - Loss of Lock in PLL Reset");
    if (RCM_SRS0 & RCM_SRS0_LVD_MASK) port->println("[RCM_SRS0] - Low-voltage Detect Reset");
    if (RCM_SRS0 & RCM_SRS0_WAKEUP_MASK) {
        port->println("[RCM_SRS0] Wakeup bit set from low power mode ");
        if ((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 3) port->println("[SMC_PMCTRL] - LLS exit ") ;
        if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 0)) port->println("[SMC_PMCTRL] - VLLS0 exit ") ;
        if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 1)) port->println("[SMC_PMCTRL] - VLLS1 exit ") ;
        if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 2)) port->println("[SMC_PMCTRL] - VLLS2 exit") ;
        if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 3)) port->println("[SMC_PMCTRL] - VLLS3 exit ") ;
    }
    if (SMC_PMSTAT == 0x01) port->println("[SMC_PMSTAT] - Current Power Mode RUN") ;
    if (SMC_PMSTAT == 0x02) port->println("[SMC_PMSTAT] - Current Power Mode STOP") ;
    if (SMC_PMSTAT == 0x04) port->println("[SMC_PMSTAT] - Current Power Mode VLPR") ;
    if (SMC_PMSTAT == 0x08) port->println("[SMC_PMSTAT] - Current Power Mode VLPW") ;
    if (SMC_PMSTAT == 0x10) port->println("[SMC_PMSTAT] - Current Power Mode VLPS") ;
    if (SMC_PMSTAT == 0x20) port->println("[SMC_PMSTAT] - Current Power Mode LLS") ;
    if (SMC_PMSTAT == 0x40) port->println("[SMC_PMSTAT] - Current Power Mode VLLS") ;
    port->println("------------------------------------------");
}
/***************************** PRIVATE: ******************************/
bool TEENSY3_LP::sleepHandle(const char* caller, uint32_t wakeType, uint32_t var1, uint16_t var2) {
    
    boolean error = false;
    
    if (wakeType == 0) {
        error = true;
        return error;
    }
    
    bool deepSlpCall = strncmp(caller, "DeepSleep", 9);
    
    bool hibernateCall = strncmp(caller, "Hibernate", 9);
 
    if (wakeType & LPTMR_WAKE) {
        stopflag |= LPTMR_WAKE;
        lptmrHandle(var1);
    }
    if (wakeType & GPIO_WAKE) {
        //gpioHandle(var1, PIN_ANY);
    }
    if (wakeType & RTCA_WAKE) {
        stopflag |= RTCA_WAKE;
        rtcHandle(var1);
    }
    if (wakeType & TSI_WAKE) {
        stopflag |= TSI_WAKE;
        tsiHandle(var1, var2);
    }
    if (wakeType & CMP0_WAKE) {
        stopflag |= CMP0_WAKE;
        cmpHandle();
    }
    
    llwu_configure(var1,PIN_ANY, wakeType);
    
    NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
    
    if (!deepSlpCall) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            lowLeakageSource = LLS;
            enter_lls();// enter lls sleep mode
        }
    }
    if (!hibernateCall) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            lowLeakageSource = VLLS;
            enter_vlls3();// enter vlls3 sleep mode
        }
    }
    
    return false;
}

void TEENSY3_LP::gpioHandle(uint32_t pin, uint8_t pinType) {

}

void TEENSY3_LP::lptmrHandle(uint32_t timeout) {
    lptmr_init();
    lptmr_start(timeout);// start timer in msec
}

void TEENSY3_LP::rtcHandle(unsigned long unixSec) {
    rtc_alarm(unixSec);// alarm in secs
}

void TEENSY3_LP::cmpHandle(void) {
    pinMode(11, INPUT);
    pinMode(12, INPUT);
    cmp_init();
    // TODO: enable Comparator (CMP) wakeup
}

void TEENSY3_LP::tsiHandle(uint8_t var, uint16_t threshold) {
    tsi_init(var, threshold);// tsi pin and wake threshold
}

