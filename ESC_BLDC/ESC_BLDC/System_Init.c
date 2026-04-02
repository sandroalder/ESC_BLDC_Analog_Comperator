/*
 * System_Init.c
 *
 * Created: 01.09.2023 15:11:52
 * Author : SA061401
 */ 

#include <avr/io.h>
#include "System_Init.h"


void clock_init (void)
{
	// Ermögliche das schreiben in geschützte Register
	CPU_CCP = CCP_IOREG_gc;
	// Wähle clock source
	CLKCTRL.MCLKCTRLA = CLK_SEL;
	// Ermögliche das schreiben in geschützte Register
	CPU_CCP = CCP_IOREG_gc;
	// Prescaler aus oder ein
	if (CLK_PRSC)
		CLKCTRL.MCLKCTRLB |=  CLKCTRL_PEN_bm;
	else
		CLKCTRL.MCLKCTRLB &=  ~CLKCTRL_PEN_bm;	
	// Warte bis sich der clock umgestellt hat
	while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm){}
}
