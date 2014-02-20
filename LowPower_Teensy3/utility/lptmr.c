/*******************************************************************************
 *  lptmr.c
 *  Teensy3
 *
 * Purpose:     Provides routines for the Low-Power Timer.
 *
 * NOTE:        This Timer will work across all power modes. It can also
 *              be used as a wakeup source from low power modes. 
 *
 *              Not very accurate need to work on configuration
 *******************************************************************************/
#include "lptmr.h"

void lptmr_start(float period) {
    /* SIM_SCGC5: LPTMR=1 */
    SIM_SCGC5 |= SIM_SCGC5_LPTIMER;
    /* LPTMR0_PSR: PRESCALE=0,PBYP=0,PCS=1 */
    LPTMR0_PSR = LPTMR_PSR_PCS(0x01) | LPTMR_PSR_PBYP_MASK;
    /* LPTMR0_CSR: TCF=0,TIE=0,TPS=0,TPP=0,TFC=0,TMS=0,TEN=0 */
    LPTMR0_CSR = 0x00U;
    /* LPTMR0_CSR: TCF=1,TIE=1,TPS=0,TPP=0,TFC=0,TMS=0,TEN=0 */
    //LPTMR0_CSR = (LPTMR_CSR_TCF_MASK | LPTMR_CSR_TIE_MASK);
    LPTMR0_CSR |= LPTMR_CSR_TIE_MASK;
    // TODO: what clock is this using?
    /* LPTMR0_CMR: COMPARE=period(ms) */
    LPTMR0_CMR = LPTMR_CMR_COMPARE(period);
    LPTMR0_CSR = (uint32_t)((LPTMR0_CSR & (uint32_t)~(uint32_t)( LPTMR_CSR_TCF_MASK)) | (uint32_t)(LPTMR_CSR_TEN_MASK));
    //LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;
}

void lptmr_stop(void) {
    LPTMR0_CSR |= LPTMR_CSR_TCF_MASK;
    LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;
}
