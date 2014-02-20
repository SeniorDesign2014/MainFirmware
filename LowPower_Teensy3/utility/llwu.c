/*******************************************************************************
 *  llwu.c
 *  Teensy3
 *
 * Purpose:     Provides routines for configuring low power wake sources.
 *
 * Notes:       Pin mapping is done automatically so just passing the
 *              digital pin name is sufficient to enable that pin to
 *              wake the teensy from sleep. 
 *              Digital pins that can be used: 22,21,16,11,10,9,7,6,4,2
 *              
 *              Modules that can currently wake the teensy are LPTMR -
 *              (Low-Power Timer). RTC will be implemented in the future.
 *******************************************************************************/

#include "llwu.h"

volatile  uint32_t flags;

/*******************************************************************************
 *
 *       llwu_reset_enable -
 *
 *******************************************************************************/
// TODO: Assuming this will keep mcu in sleep mode after reset?
void llwu_reset_enable(void) {
    //LLWU_RST = LLWU_RST_LLRSTE_MASK;   //no reset filter for now
}


/*******************************************************************************
 *
 *       llwu_configure -
 *
 * description: Set up the LLWU for wakeup of the MCU from LLS and VLLSx modes
 * from the selected pin or module.
 *
 * inputs:  pin_en - indicates the wakeup pin is enabled.
 *          rise_fall - 0x00 = External input disabled as wakeup
 *          0x01 - External input enabled as rising edge detection
 *          0x02 - External input enabled as falling edge detection
 *          0x03 - External input enabled as any edge detection
 *          module_en - indicates the wakeup module is enabled.
 *******************************************************************************/
void llwu_configure(uint32_t pin_en, uint8_t rise_fall, uint32_t module_en ) {
    llwu_clear_flags();
    LLWU_PE2 = 0;
    if( pin_en & LLWU_PIN_4) {
        LLWU_PE2 |= LLWU_PE2_WUPE4(rise_fall);
    }
    if( pin_en & LLWU_PIN_16) {  //TSI enabled
        LLWU_PE2 |= LLWU_PE2_WUPE5(rise_fall);
    }
    if( pin_en & LLWU_PIN_22) {// TSI ensbled
        LLWU_PE2 |= LLWU_PE2_WUPE6(3);
    }
    if( pin_en & LLWU_PIN_9) {
        LLWU_PE2 |= LLWU_PE2_WUPE7(rise_fall);
    }
    
    LLWU_PE3 = 0;
    if( pin_en & LLWU_PIN_10) {
        LLWU_PE3 |= LLWU_PE3_WUPE8(rise_fall);
    }
    if( pin_en & LLWU_PIN_11) {
        LLWU_PE3 |= LLWU_PE3_WUPE10(rise_fall);
    }
    
    LLWU_PE4 = 0;
    if( pin_en & LLWU_PIN_2) {
        LLWU_PE4 |= LLWU_PE4_WUPE12(rise_fall);
    }
    if( pin_en & LLWU_PIN_7) {
        LLWU_PE4 |= LLWU_PE4_WUPE13(rise_fall);
    }
    if( pin_en & LLWU_PIN_6) {
        LLWU_PE4 |= LLWU_PE4_WUPE14(rise_fall);
    }
    if( pin_en & LLWU_PIN_21) {
        LLWU_PE4 |= LLWU_PE4_WUPE15(rise_fall);
    }
    
    LLWU_ME = 0;
    if( module_en & LLWU_LPTMR_MOD) {
        LLWU_ME |= LLWU_ME_WUME0_MASK;
    }
    if( module_en & LLWU_CMP0_MOD) {
        LLWU_ME |= LLWU_ME_WUME1_MASK;
    }
    if( module_en & LLWU_CMP1_MOD) {
        LLWU_ME |= LLWU_ME_WUME2_MASK;
    }
    if( module_en & LLWU_TSI_MOD) {
        LLWU_ME |= LLWU_ME_WUME4_MASK;
    }
    if( module_en & LLWU_RTCA_MOD) {
        LLWU_ME |= LLWU_ME_WUME5_MASK;
    }
    if( module_en & LLWU_RTCS_MOD) {
        LLWU_ME |= LLWU_ME_WUME7_MASK;
    }
}


/*******************************************************************************
 *
 *       llwu_configure_filter -
 *
 *******************************************************************************/
void llwu_configure_filter(unsigned int wu_pin_num, unsigned char filter_en, unsigned char rise_fall ) {
    //wu_pin_num is the pin number to be written to FILTSEL.  wu_pin_num is not the same as pin_en.
    uint8_t temp;
    
    temp = 0;
    //first clear filter values and clear flag by writing a 1
    LLWU_FILT1 = LLWU_FILT1_FILTF_MASK;
    LLWU_FILT2 = LLWU_FILT2_FILTF_MASK;
    
    if(filter_en == 1) {
        //clear the flag bit and set the others
        temp |= (LLWU_FILT1_FILTF_MASK) | (LLWU_FILT1_FILTE(rise_fall) | LLWU_FILT1_FILTSEL(wu_pin_num));
        LLWU_FILT1 = temp;
    }else if (filter_en == 2) {
        //clear the flag bit and set the others
        temp |= (LLWU_FILT2_FILTF_MASK) | (LLWU_FILT2_FILTE(rise_fall) | LLWU_FILT2_FILTSEL(wu_pin_num));
        LLWU_FILT2 = temp;
    }else {
        
    }
}

/*******************************************************************************
 *
 *       llwu_clear_flags -
 *
 * description: Clear wakeup flags.
 *
 *******************************************************************************/
uint32_t llwu_clear_flags(void) {
    flags = (LLWU_F1 | LLWU_F2<<8 | LLWU_F3<<16);
    if (LLWU_F1 & LLWU_F1_WUF0_MASK) LLWU_F1 |= LLWU_F1_WUF0_MASK;// write one to clear the flag
    if (LLWU_F1 & LLWU_F1_WUF1_MASK) LLWU_F1 |= LLWU_F1_WUF2_MASK;// write one to clear the flag
    if (LLWU_F1 & LLWU_F1_WUF2_MASK) LLWU_F1 |= LLWU_F1_WUF2_MASK;// write one to clear the flag
    if (LLWU_F1 & LLWU_F1_WUF3_MASK) LLWU_F1 |= LLWU_F1_WUF3_MASK;// write one to clear the flag
    if (LLWU_F1 & LLWU_F1_WUF4_MASK) LLWU_F1 |= LLWU_F1_WUF4_MASK;// write one to clear the flag
    if (LLWU_F1 & LLWU_F1_WUF5_MASK) LLWU_F1 |= LLWU_F1_WUF5_MASK;// write one to clear the flag
    if (LLWU_F1 & LLWU_F1_WUF6_MASK) LLWU_F1 |= LLWU_F1_WUF6_MASK;// write one to clear the flag
    if (LLWU_F1 & LLWU_F1_WUF7_MASK) LLWU_F1 |= LLWU_F1_WUF7_MASK;// write one to clear the flag
    if (LLWU_F2 & LLWU_F2_WUF8_MASK) LLWU_F2 |= LLWU_F2_WUF8_MASK;// write one to clear the flag
    if (LLWU_F2 & LLWU_F2_WUF9_MASK) LLWU_F2 |= LLWU_F2_WUF9_MASK;// write one to clear the flag
    if (LLWU_F2 & LLWU_F2_WUF10_MASK) LLWU_F2 |= LLWU_F2_WUF10_MASK;// write one to clear the flag
    if (LLWU_F2 & LLWU_F2_WUF11_MASK) LLWU_F2 |= LLWU_F2_WUF11_MASK;// write one to clear the flag
    if (LLWU_F2 & LLWU_F2_WUF12_MASK) LLWU_F2 |= LLWU_F2_WUF12_MASK;// write one to clear the flag
    if (LLWU_F2 & LLWU_F2_WUF13_MASK) LLWU_F2 |= LLWU_F2_WUF13_MASK;// write one to clear the flag
    if (LLWU_F2 & LLWU_F2_WUF14_MASK) LLWU_F2 |= LLWU_F2_WUF14_MASK;// write one to clear the flag
    if (LLWU_F2 & LLWU_F2_WUF15_MASK) LLWU_F2 |= LLWU_F2_WUF15_MASK;// write one to clear the flag
    if (LLWU_F3 & LLWU_F3_MWUF0_MASK) LLWU_F3 |= LLWU_F3_MWUF0_MASK;// write one to clear the flag
    if (LLWU_F3 & LLWU_F3_MWUF1_MASK) LLWU_F3 |= LLWU_F3_MWUF1_MASK;// write one to clear the flag
    if (LLWU_F3 & LLWU_F3_MWUF2_MASK) LLWU_F3 |= LLWU_F3_MWUF2_MASK;// write one to clear the flag
    if (LLWU_F3 & LLWU_F3_MWUF3_MASK) LLWU_F3 |= LLWU_F3_MWUF3_MASK;// write one to clear the flag
    if (LLWU_F3 & LLWU_F3_MWUF4_MASK) LLWU_F3 |= LLWU_F3_MWUF4_MASK;// write one to clear the flag
    if (LLWU_F3 & LLWU_F3_MWUF5_MASK) LLWU_F3 |= LLWU_F3_MWUF5_MASK;// write one to clear the flag
    if (LLWU_F3 & LLWU_F3_MWUF6_MASK) LLWU_F3 |= LLWU_F3_MWUF6_MASK;// write one to clear the flag
    if (LLWU_F3 & LLWU_F3_MWUF7_MASK) LLWU_F3 |= LLWU_F3_MWUF7_MASK;// write one to clear the flag
    
    if(LLWU_FILT1 & LLWU_FILT1_FILTF_MASK) LLWU_FILT1 |= LLWU_FILT1_FILTF_MASK;// write one to clear the flag
    if(LLWU_FILT2 & LLWU_FILT2_FILTF_MASK) LLWU_FILT2 |= LLWU_FILT2_FILTF_MASK;// write one to clear the flag
    return flags;
}

