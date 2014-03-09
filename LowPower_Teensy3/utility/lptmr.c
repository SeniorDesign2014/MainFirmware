/*******************************************************************************
 *  lptmr.c
 *  Teensy3
 *
 * Purpose:     Provides routines for the Low-Power Timer.
 *
 * NOTE:        This Timer will work across all power modes. It can also
 *              be used as a wakeup source from low power modes. 
 *
 *              Updated Clock configuration, now sub millisecond intervals
 *              are supported. Credit: Kyle Matthews - kyle.matthews@duke.edu
 *******************************************************************************/
#include "lptmr.h"
#include "bitband.h"

#define SIM_SCGC5_LPTIMER_BIT 0x00

void lptmr_init(void) {
    // check clocks
    if (!BITBAND_U32(SIM_SCGC5, SIM_SCGC5_LPTIMER_BIT)) { BITBAND_U32(SIM_SCGC5, SIM_SCGC5_LPTIMER_BIT) = 1; }
    LPTMR0_CSR=0x00;
    LPTMR0_PSR=0x00;
    LPTMR0_CMR=0x00;
    // Ensure Internal Reference Clock is Enabled
    MCG_C1 |= MCG_C1_IRCLKEN;
    //Enable fast internal ref clock by setting MCG_C2[IRCS]=1
    //If wanted to use 32Khz slow mode, set MCG_C2[IRCS]=0 instead
    MCG_C2 &= ~MCG_C2_IRCS;
    // Use RTC 32.768kHZ oscillator
    SIM_SOPT1 |= SIM_SOPT1_OSC32KSEL(0x03);
    // External reference stop enable - OSCERCLK remains enabled when MCU enters stop modes
    // Enable external reference clock (OSCERCLK), PAGE 533
    OSC0_CR |= (OSC_EREFSTEN | OSC_ERCLKEN);
    // select Osillator as External Reference source.
    MCG_C2 |= MCG_C2_EREFS;
    // set Prescale
    LPTMR0_PSR = (LPTMR_PSR_PBYP_MASK | LPTMR_PSR_PCS(0x01));
    // set Control Status Register
    LPTMR0_CSR = (LPTMR_CSR_TCF_MASK | LPTMR_CSR_TIE_MASK);
}

void lptmr_start(uint32_t period) {
    LPTMR0_CMR = period;
    LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;
}

void lptmr_stop(void) {
    LPTMR0_CSR |= LPTMR_CSR_TCF_MASK;
    LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;
    if (BITBAND_U32(SIM_SCGC5, SIM_SCGC5_LPTIMER_BIT)) { BITBAND_U32(SIM_SCGC5, SIM_SCGC5_LPTIMER_BIT) = 0; }
}
