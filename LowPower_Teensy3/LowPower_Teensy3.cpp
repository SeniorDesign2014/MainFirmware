/*******************************************************************************
 *  LowPower_Teensy3.cpp
 *  Teensy3
 *
 * Purpose:    Provides routines for configuring the Teensy3 for low power.
 *
 * NOTE:       None
 *******************************************************************************/

#include "LowPower_Teensy3.h"

volatile uint32_t wakeSource;// hold llwu wake up source for wakeup isr
volatile uint32_t stopflag;// hold module wake up sources for wakeup isr
volatile uint8_t lowLeakageSource;// hold lowleakage mode for wakeup isr

TEENSY3_LP::TEENSY3_LP() {
    llwu_clear_flags();// clear llwu flags
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
    // clear usb regulator standby bit in VLPR mode, no standby
    SIM_SOPT1 &= ~SIM_SOPT1_USBVSTBY_MASK;
}

/****************************** Sleep *******************************
 * Routines to enable different sleep modes on the teensy3.
 *
 **wake_isr() - 
 *      handles wake ups from LLS or VLLS. It clears LLWU flags
 *      sets MCG_C1 to PEE run mode clears flags and sets wakeup
 *      variable.
 *
 **RUN(uint8_t mode, uint8_t woi) - 
 *      transition into BLPI mode which configures the core(2 MHZ), 
 *      bus(2 MHZ), flash(1 MHZ) clocks and configures SysTick for 
 *      the reduced freq, then enter into vlpr mode. Exiting Low Power 
 *      Run mode will transition to PEE mode and reconfigures clocks
 *      and SysTick to a pre RUN state and exit vlpr to normal run.
 * Parameter:  mode -> LP_RUN_ON = enter Low Power Run(VLPR)
 *             mode -> LP_RUN_OFF = exit Low Power Run(VLPR)
 *             woi  -> NO_WAKE_ON_INTERRUPT = no exit LPR on interrupt 
 *             woi  -> WAKE_ON_INTERRUPT = exit LPR on interrupt
 *
 * WAIT() - todo... Configure wait mode.
 * 
 * Sleep() - todo... Configure stop mode.
 * 
 **DeepSleep(uint8_t wakeType, uint32_t var, uint16_t var2) - 
 *      set teensy3 to LLS sleep mode. Exited by Low-Power Timer, 
 *      Digital Pin, RTC and TSI; Code execution begins after call 
 *      to DeepSleep function.
 * Parameters:  wakeType -> LPTMR_WAKE, GPIO_WAKE, RTCA_WAKE, TSI_WAKE defines
 *              var -> LPTMR_WAKE = timeout in msec
 *              var -> GPIO = wake pin, which are defined in the header
 *              var -> RTCA = alarm in secs
 *              var -> TSI_WAKE = wake pin, only one pin can selected
 *              var2 -> TSI wakeup threshold
 *
 **DeepSleep(volatile struct configSleep* config) -
 *      uses the struct to configure the teensy wake sources. This
 *      allows the teensy to have mulitple wakeup sources and  
 *      mulitple configurations.
 * Parameters:  configSleep* struct - defined in header
 *
 *
 **Hibernate(uint8_t wakeType, uint32_t var, uint16_t var2) - 
 *      set teensy3 to VLLS3 sleep mode. Exited by Low-Power Timer,
 *      Digital Pin, RTC and TSI; Code execution begins after call
 *      to DeepSleep function. Code execution begins through reset flow.
 * Parameters:  wakeType -> LPTMR_WAKE, GPIO_WAKE, RTCA_WAKE, TSI_WAKE defines
 *              var -> LPTMR_WAKE = timeout in msec
 *              var -> GPIO = wake pin, which are defined in the header
 *              var -> RTCA = alarm in secs
 *              var -> TSI_WAKE = wake pin, only one pin can selected
 *              var2 -> TSI wakeup threshold
 *
 **Hibernate(volatile struct configSleep* config) -
 *      uses the struct to configure the teensy wake sources. This
 *      allows the teensy to have mulitple wakeup sources and
 *      mulitple configurations.
 * Parameters:  configSleep* struct - defined in header
 *
 *
 **PrintSRS() - 
 *      prints the reset type and current power mode.
 *
 *
 * Note:   This is work in progress, not sure of all the implications
 *         on teensy functionality when using sleep mode routines. 
 ********************************************************************/
void wakeup_isr(void) {
    wakeSource = llwu_clear_flags();// clear llwu flags after wakeup a/ store wakeup source
    pbe_pee();// mcu is in PBE from LLS wakeup, transition back to PEE
    if (stopflag & LPTMR_WAKE && lowLeakageSource == LLS) {
        lptmr_stop();
    }
    if (stopflag & RTCA_WAKE && lowLeakageSource == LLS) {
        rtc_stop();
    }
     if (stopflag & TSI_WAKE && lowLeakageSource == LLS) {
        tsi_stop();
    }
    NVIC_DISABLE_IRQ(IRQ_LLWU); // disable wakeup isr
}

/***************************** PUBLIC: ******************************/
void TEENSY3_LP::Run(uint8_t mode) {
    Run(mode, 0);
}

void TEENSY3_LP::Run(uint8_t mode, uint8_t woi) {
    if (mode == LP_RUN_ON) {
        pee_blpi();// transition from PEE to BLPI
        enter_vlpr(woi);// enter low power Run
    }
    if (mode == LP_RUN_OFF) {
        exit_vlpr();// exit low power Run
        blpi_pee();// transition from BLPI to PEE
    }
}

void TEENSY3_LP::Wait(void) {
    // TODO: Not working yet
}

void TEENSY3_LP::Sleep(void) {
    // TODO: Not working yet
}

void TEENSY3_LP::DeepSleep(uint32_t wakeType, uint32_t var, uint16_t var2) {
    lowLeakageSource = LLS;
    stopflag = 0;
    if (wakeType & LPTMR_WAKE) {
        stopflag |= LPTMR_WAKE;
        lptmrHandle(var);
        llwu_configure(0, 0, wakeType);
        NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
        enter_lls();// enter lls sleep mode
    }
    if (wakeType & GPIO_WAKE) {
        gpioHandle(var, PIN_ANY);
        llwu_configure(var, PIN_ANY, wakeType);
        NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
        enter_lls();// enter lls sleep mode
    }
    if (wakeType & RTCA_WAKE) {
        stopflag |= RTCA_WAKE;
        rtcHandle(var);
        llwu_configure(0, 0, wakeType);
        NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
        enter_lls();// enter lls sleep mode
    }
    if (wakeType & TSI_WAKE) {
        stopflag |= TSI_WAKE;
        tsiHandle(var, var2);
        llwu_configure(0, 0, wakeType);
        NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
        enter_lls();// enter lls sleep mode
    }
}

void TEENSY3_LP::DeepSleep(uint32_t wakeType, uint32_t var) {
    DeepSleep(wakeType, var, 0);
}

void TEENSY3_LP::DeepSleep(volatile struct configSleep* config) {
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

void TEENSY3_LP::Hibernate(uint32_t wakeType, uint32_t var, uint16_t var2) {  
    lowLeakageSource = VLLS;
    if (wakeType & LPTMR_WAKE) {
        lptmrHandle(var);
        llwu_configure(0, 0, wakeType);
        NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
        enter_vlls3();// enter vlls3 sleep mode
    }
    if (wakeType & GPIO_WAKE) {
        gpioHandle(var, PIN_ANY);
        llwu_configure(var, PIN_ANY, wakeType);
        NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
        enter_vlls3();// enter vlls3 sleep mode
    }
    if (wakeType & RTCA_WAKE) {
        rtcHandle(var);
        llwu_configure(0, 0, wakeType);
        NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
        enter_vlls3();// enter lls sleep mode
    }
    if (wakeType & TSI_WAKE) {
        tsiHandle(var, var2);
        llwu_configure(0, 0, wakeType);
        NVIC_ENABLE_IRQ(IRQ_LLWU);// enable llwu isr
        enter_vlls3();// enter lls sleep mode
    }
}

void TEENSY3_LP::Hibernate(uint32_t wakeType, uint32_t var) {
    Hibernate(wakeType, var, 0);
}

void TEENSY3_LP::Hibernate(volatile struct configSleep* config) {
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
    
    enter_vlls3();// enter vlls3 sleep mode
}

void TEENSY3_LP::PrintSRS(void) {
    Serial.println("------------------------------------------");
    if (RCM_SRS1 & RCM_SRS1_SACKERR_MASK) Serial.println("[RCM_SRS0] - Stop Mode Acknowledge Error Reset");
    if (RCM_SRS1 & RCM_SRS1_MDM_AP_MASK) Serial.println("[RCM_SRS0] - MDM-AP Reset");
    if (RCM_SRS1 & RCM_SRS1_SW_MASK) Serial.println("[RCM_SRS0] - Software Reset");
    if (RCM_SRS1 & RCM_SRS1_LOCKUP_MASK) Serial.println("[RCM_SRS0] - Core Lockup Event Reset");
    if (RCM_SRS0 & RCM_SRS0_POR_MASK) Serial.println("[RCM_SRS0] - Power-on Reset");
    if (RCM_SRS0 & RCM_SRS0_PIN_MASK) Serial.println("[RCM_SRS0] - External Pin Reset");
    if (RCM_SRS0 & RCM_SRS0_WDOG_MASK) Serial.println("[RCM_SRS0] - Watchdog(COP) Reset");
    if (RCM_SRS0 & RCM_SRS0_LOC_MASK) Serial.println("[RCM_SRS0] - Loss of External Clock Reset");
    if (RCM_SRS0 & RCM_SRS0_LOL_MASK) Serial.println("[RCM_SRS0] - Loss of Lock in PLL Reset");
    if (RCM_SRS0 & RCM_SRS0_LVD_MASK) Serial.println("[RCM_SRS0] - Low-voltage Detect Reset");
    if (RCM_SRS0 & RCM_SRS0_WAKEUP_MASK) {
        Serial.println("[RCM_SRS0] Wakeup bit set from low power mode ");
        if ((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 3) Serial.println("[SMC_PMCTRL] - LLS exit ") ;
        if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 0)) Serial.println("[SMC_PMCTRL] - VLLS0 exit ") ;
        if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 1)) Serial.println("[SMC_PMCTRL] - VLLS1 exit ") ;
        if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 2)) Serial.println("[SMC_PMCTRL] - VLLS2 exit") ;
        if (((SMC_PMCTRL & SMC_PMCTRL_STOPM_MASK)== 4) && ((SMC_VLLSCTRL & SMC_VLLSCTRL_VLLSM_MASK)== 3)) Serial.println("[SMC_PMCTRL] - VLLS3 exit ") ;
    }
    if (SMC_PMSTAT == 0x01) Serial.println("[SMC_PMSTAT] - Current Power Mode RUN") ;
    if (SMC_PMSTAT == 0x02) Serial.println("[SMC_PMSTAT] - Current Power Mode STOP") ;
    if (SMC_PMSTAT == 0x04) Serial.println("[SMC_PMSTAT] - Current Power Mode VLPR") ;
    if (SMC_PMSTAT == 0x08) Serial.println("[SMC_PMSTAT] - Current Power Mode VLPW") ;
    if (SMC_PMSTAT == 0x10) Serial.println("[SMC_PMSTAT] - Current Power Mode VLPS") ;
    if (SMC_PMSTAT == 0x20) Serial.println("[SMC_PMSTAT] - Current Power Mode LLS") ;
    if (SMC_PMSTAT == 0x40) Serial.println("[SMC_PMSTAT] - Current Power Mode VLLS") ;
    Serial.println("------------------------------------------");
}

/***************************** PRIVATE: ******************************/
void TEENSY3_LP::gpioHandle(uint32_t pin, uint8_t pinType) {
    Serial.send_now();// send any pending usb data
    delay(5);
    Serial.flush();// flush USB
    delay(5);
}

void TEENSY3_LP::lptmrHandle(float timeout) {
    Serial.send_now();// send any pending usb data
    delay(5);
    Serial.flush();// flush USB
    delay(5);
    lptmr_start(timeout);// start timer in msec
}

void TEENSY3_LP::rtcHandle(unsigned long unixSec) {
    Serial.send_now();// send any pending usb data
    delay(5);
    Serial.flush();// flush USB
    delay(5);
    rtc_alarm(unixSec);// alarm in secs
}

void TEENSY3_LP::cmpHandle(void) {
    // TODO: enable Comparator (CMP) wakeup
}

void TEENSY3_LP::tsiHandle(uint8_t var, uint16_t threshold) {
    Serial.send_now();// send any pending usb data
    delay(5);
    Serial.flush();// flush USB
    delay(5);
    tsi_sleep(var, threshold);// tsi pin and wake threshold
}

