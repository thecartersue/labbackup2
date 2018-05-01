// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 4/10/2016
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdio.h>
#include <stdint.h>
#include "ADC.h"
#include "tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none

void ADC_Init(void){
	volatile int delay = 0;
	SYSCTL_RCGCADC_R |= 0x0001;
  delay++;delay++;delay++;delay++;delay++;
	 delay++;delay++;delay++;delay++;delay++;
	SYSCTL_RCGCGPIO_R |= 0x08;
///while((SYSCTL_PRGPIO_R&0x04) != 0x04){};
  delay++;delay++;delay++;delay++;delay++;
	 delay++;delay++;delay++;delay++;delay++;
	GPIO_PORTD_DIR_R &= ~0x04;    
  GPIO_PORTD_AFSEL_R |= 0x04;    
  GPIO_PORTD_DEN_R &= ~0x04;   
  GPIO_PORTD_AMSEL_R |= 0x04;     
//while((SYSCTL_PRADC_R&0x0001) != 0x0001){};
  delay++;delay++;delay++;delay++;delay++;
	 delay++;delay++;delay++;delay++;delay++;

 ADC0_PC_R &= ~0xF;              // 7) clear max sample rate field
 ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
 ADC0_SSPRI_R = 0x0123;          // 8) Sequencer 3 is highest priority
 ADC0_ACTSS_R &= ~0x0008;        // 9) disable sample sequencer 3
 ADC0_EMUX_R &= ~0xF000;         // 10) seq3 is software trigger
 ADC0_SSMUX3_R &= ~0x000F;       // 11) clear SS3 field
 ADC0_SSMUX3_R += 5;             //    set channel
 ADC0_SSCTL3_R = 0x0006;         // 12) no TS0 D0, yes IE0 END0
 ADC0_IM_R &= ~0x0008;           // 13) disable SS3 interrupts
 ADC0_ACTSS_R |= 0x0008;         // 14) enable sample sequencer 3
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
uint32_t ADC_In(void){  
 uint32_t result;
 ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
 while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
// if you have an A0-A3 revision number, you need to add an 8 usec wait here
 result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
 ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
 return result;	
}
