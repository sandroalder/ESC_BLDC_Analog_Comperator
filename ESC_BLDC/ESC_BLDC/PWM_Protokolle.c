/*
 * PWM_Protokolle.c
 *
 * Created: 05.02.2026 12:23:09
 *  Author: SA061401
 */ 


// PWM Kommunikation Multishot / Oneshot42 / Oneshot125

// Hier die Pinbelegung eintragen

/////////////////////////////////////////////////////////////////

// Konfiguriere hier die Parameter

// mit prescaler 1 ein increment alle 0.05uS bei einem 20MHz Takt / max. Pulse 3’276.8uS
#define CLK_select	TCB_CLKSEL_CLKDIV1_gc			// für d_shot150

#define TCBx		TCB3							// Timer counterB3
#define	TCBx_vect	TCB3_INT_vect
#define USERTCBx	USERTCB3						// Timer counterB3

#define CHANNELx	EVSYS.CHANNEL0					// Channel 0
#define EV_CHANNEL	EVSYS_CHANNEL_CHANNEL0_gc		// Channel 0

#define PORTx_PINx	EVSYS_GENERATOR_PORT0_PIN7_gc	// PortA Pin7
//#define PORTx_PINx	EVSYS_GENERATOR_PORT1_PIN2_gc	// PortB Pin2



#include "PWM_Protokolle.h"

#include <inttypes.h>
#include <stdlib.h>
#include <avr/interrupt.h>


uint16_t PWM_raw_read (void)
{
	return TCBx.CCMP;
}


uint16_t static zero_pos = 100;
float static scaling_2 = 2;
float static scaling_5 = 5;
uint16_t static scaling_2_Zaehler;		// Rechne mit Zähler und Nenner, da um Längen effizienter als float
uint16_t static scaling_5_Zaehler;
#define scaling_Nenner >> 8				// Statt Bruch einfach mal 256 und dann 8 nach rechts schieben

void PWM_calibrate (uint16_t raw_pulse)
{
	zero_pos = raw_pulse;
    scaling_2 = 2047.0f / ((2.0f * zero_pos) - zero_pos);
    scaling_5 = 2047.0f / ((5.0f * zero_pos) - zero_pos);
	scaling_2_Zaehler = (float)scaling_2 * 256;
	scaling_5_Zaehler = (float)scaling_5 * 256;
}

uint16_t Multishot_read (void)
{
    uint32_t tmp;

    cli();
    tmp = TCBx.CCMP;
    sei();

    if (tmp > zero_pos)
		tmp -= zero_pos;
    else
		tmp = 0;

    //tmp *= (float)scaling_5;
	tmp = (tmp * scaling_5_Zaehler) scaling_Nenner;		// shift >> 8 statt Bruch
	
    if (tmp > 2047)
		tmp = 2047;

    return tmp;
}

uint16_t Oneshot42_read (void)
{
	uint16_t tmp = 0;
	return tmp;
}



void PWM_shot_init (void)
{
	// Setze Port als input event
	CHANNELx = PORTx_PINx;
	// Verbinde zu event channel 0
	EVSYS.USERTCBx = EV_CHANNEL;
	
	// Aktviere TCB ohne Prescaler
	TCBx.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
	
	// Messe Puls
	TCBx.CTRLB = TCB_CNTMODE_PW_gc;
	
	// Setze Capture interrupt
	// TCBx.INTCTRL = TCB_CAPT_bm;
	
	// Setze Event Input und triggere auf positiven Puls
	TCBx.EVCTRL	|= TCB_CAPTEI_bm;
	TCBx.EVCTRL	&= ~TCB_EDGE_bm;
}

void PWM_shot_stop (void)
{
	 TCBx.CTRLA  &= ~TCB_ENABLE_bm;
}