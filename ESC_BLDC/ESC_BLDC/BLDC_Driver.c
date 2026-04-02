/*
 * BLDC_Driver.c
 *
 * Created: 02.02.2026 13:58:12
 * Author : SA061401
 */ 

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#define	F_CPU 20000000UL
#include <util/delay.h>

#include "BLDC_Driver.h"
#include "median_filter.h"

uint8_t reverse = 0;

/////////////////////////////////////////////////////////////////
// Motor_Driver

// setze reverse Flag
// das reverse Flag ersetzt einfach alles von Phase_B mit Phase_C und anders herum
void BLDC_set_reverse (uint8_t reverse_flag)
{
	reverse = reverse_flag;
}

// Init PWM
void BLDC_PWM_Init (void)
{
	//config Timer
	
	// PORTB als Ausgang
	PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTB_gc;
		
	/* Non Split mode */
	// select prescaler
	// Ohne prescaler ein increment alle 0.05uS bei einem 20MHz Takt
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV1_gc;
	
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
	// Aktiviere compare match für den PWM
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;
	
	// setze duty cycle auf 25uS / macht 40kHz Regelfrequenz
	TCA0.SINGLE.PERBUF = 511;	// mit 511 sind es 39.22 kHz	
	// compare match auf 0
	TCA0.SINGLE.CMP0BUF = 0;
	
	// starte Timer
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

void BLDC_PWM_Set (uint16_t PWM_Wert)
{
	TCA0.SINGLE.CMP0BUF = PWM_Wert;
}

void BLDC_Driver_init (void)
{
	// Init Pins
	Phase_Port.DIRSET	=	INHA;
	Phase_Port.DIRSET	=	INLA;
	Phase_Port.DIRSET	=	INHB;
	Phase_Port.DIRSET	=	INLB;
	Phase_Port.DIRSET	=	INHC;
	Phase_Port.DIRSET	=	INLC;
	
	PWM_Port.DIRSET		=	PWM;
}

// Phasen High / Low Side ein und ausschalten
static inline void Phase_A_High_off (void)	{Phase_Port.DIRSET	=	INHA;}
static inline void Phase_A_High_on (void)	{Phase_Port.DIRCLR	=	INHA;}
static inline void Phase_A_Low_off (void)	{Phase_Port.OUTCLR	=	INLA;}
static inline void Phase_A_Low_on (void)	{Phase_Port.OUTSET	=	INLA;}

static inline void Phase_B_High_off (void)	{Phase_Port.DIRSET	=	INHB;}
static inline void Phase_B_High_on (void)	{Phase_Port.DIRCLR	=	INHB;}
static inline void Phase_B_Low_off (void)	{Phase_Port.OUTCLR	=	INLB;}
static inline void Phase_B_Low_on (void)	{Phase_Port.OUTSET	=	INLB;}
	
static inline void Phase_C_High_off (void)	{Phase_Port.DIRSET	=	INHC;}
static inline void Phase_C_High_on (void)	{Phase_Port.DIRCLR	=	INHC;}
static inline void Phase_C_Low_off (void)	{Phase_Port.OUTCLR	=	INLC;}
static inline void Phase_C_Low_on (void)	{Phase_Port.OUTSET	=	INLC;}
	
void BLDC_Stop (void)
{
	// Schalte alles aus
	Phase_A_High_off();
	Phase_B_High_off();
	Phase_C_High_off();
	
	Phase_A_Low_off();
	Phase_B_Low_off();
	Phase_C_Low_off();
	
	BLDC_PWM_Set (0);
}

void BLDC_Break (void)
{
	
}

// Setzt alle Output Pins auf Phase 1
uint8_t BLDC_get_ready (uint8_t Phase)
{  
	BLDC_PWM_Set (0);	// Safety falls in main vergessen
	// 6 Phasen weiter, damit alle Pins entsprechend gesetzt sind
	for (uint8_t i = 0; i < 6; i++)					
	{
		Phase = BLDC_next_Phase(Phase);
		BLDC_change_Phase(Phase);
	}
	return Phase;
}

// wechsle in nächste Phase
uint8_t BLDC_next_Phase (uint8_t Phase)
{
	if (Phase == 6)
		Phase = 1;
	else
		Phase ++;
		
	return Phase;
}

// Wechsle Phasen
void BLDC_change_Phase (uint8_t Phase)
{
	switch (Phase)
	{
		case 1:
			if (reverse == 0)
			{
				Phase_A_High_on();
				Phase_C_High_off();
			}
			else
			{
				Phase_A_High_on();
				Phase_B_High_off();	
			}
		break;
		case 2:
			if (reverse == 0)
			{
				Phase_C_Low_on();
				Phase_B_Low_off();		
			}
			else
			{
				Phase_B_Low_on();
				Phase_C_Low_off();	
			}
		break;
		case 3:
			if (reverse == 0)
			{
				Phase_B_High_on();
				Phase_A_High_off();	
			}
			else
			{
				Phase_C_High_on();
				Phase_A_High_off();	
			}
		break;
		case 4:
			if (reverse == 0)
			{
				Phase_A_Low_on();	
				Phase_C_Low_off();		
			}
			else
			{
				Phase_A_Low_on();
				Phase_B_Low_off();	
			}
		break;
		case 5:
			if (reverse == 0)
			{
				Phase_C_High_on();
				Phase_B_High_off();	
			}
			else
			{
				Phase_B_High_on();
				Phase_C_High_off();
			}
		break;
		case 6:
			if (reverse == 0)
			{
				Phase_B_Low_on();
				Phase_A_Low_off();	
			}
			else
			{
				Phase_C_Low_on();
				Phase_A_Low_off();
			}
		break;
	}
}

/////////////////////////////////////////////////////////////////
// Motor_BEMF


void BEMF_ADC_init (void)
{
	// Schalte digitale Input Buffer aus
	BEMF_Port.BEMF_Mid_PCTRL	&= ~PORT_ISC_gm;
	BEMF_Port.BEMF_Mid_PCTRL	|= PORT_ISC_INPUT_DISABLE_gc;
	BEMF_Port.BEMF_A_PCTRL		&= ~PORT_ISC_gm;
	BEMF_Port.BEMF_A_PCTRL		|= PORT_ISC_INPUT_DISABLE_gc;
	BEMF_Port.BEMF_B_PCTRL		&= ~PORT_ISC_gm;
	BEMF_Port.BEMF_B_PCTRL		|= PORT_ISC_INPUT_DISABLE_gc;
	BEMF_Port.BEMF_C_PCTRL		&= ~PORT_ISC_gm;
	BEMF_Port.BEMF_C_PCTRL		|= PORT_ISC_INPUT_DISABLE_gc;
	
	// 8 Bit mode / Aktiviere ADC
	ADC0_CTRLA	=	ADC_RESSEL_8BIT_gc
				|	ADC_ENABLE_bm;

	//  VRAFA (PD7) als Referenz / Prescaler 8
	// (clock darf nicht über eine bestimmte Frequenz sonst geht es nicht.
	// Ist im Datenblatt leider unzureichend dokumentiert)
	ADC0_CTRLC	=	ADC_REFSEL_VREFA_gc
				|	ADC_PRESC_DIV8_gc;
}

void BEMF_AC_init (void)
{
	// Nur nötig wegen Quick-Fix auf Hardware. Könnte später entfernt werden
	PORTC.PIN7CTRL &= ~PORT_ISC_gm;
	PORTC.PIN7CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTD.PIN3CTRL &= ~PORT_ISC_gm;
	PORTD.PIN3CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	PORTD.PIN0CTRL &= ~PORT_ISC_gm;
	PORTD.PIN0CTRL |= PORT_ISC_INPUT_DISABLE_gc;
	
	// Schalte digitale Input Buffer aus
	BEMF_Port_AC.BEMF_Mid_AC_PCTRL	&= ~PORT_ISC_gm;
	BEMF_Port_AC.BEMF_Mid_AC_PCTRL	|= PORT_ISC_INPUT_DISABLE_gc;
	BEMF_Port_AC.BEMF_A_AC_PCTRL	&= ~PORT_ISC_gm;
	BEMF_Port_AC.BEMF_A_AC_PCTRL	|= PORT_ISC_INPUT_DISABLE_gc;
	BEMF_Port_AC.BEMF_B_AC_PCTRL	&= ~PORT_ISC_gm;
	BEMF_Port_AC.BEMF_B_AC_PCTRL	|= PORT_ISC_INPUT_DISABLE_gc;
	BEMF_Port_AC.BEMF_C_AC_PCTRL	&= ~PORT_ISC_gm;
	BEMF_Port_AC.BEMF_C_AC_PCTRL	|= PORT_ISC_INPUT_DISABLE_gc;
	
	
	AC0.MUXCTRLA = BEMF_NEG_AC;			// setze Mid_V auf negativen Input

	// Aktiviere AC und setze ihn auf beide Flanken / Hysterese 10mV
	AC0.CTRLA = AC_ENABLE_bm | AC_INTMODE_BOTHEDGE_gc | AC_HYSMODE_10mV_gc;
	//AC0.INTCTRL = AC_CMP_bm;
}

void BLDC_AC_ignore(void)
{
	AC0.MUXCTRLA = ignore_NEG | ignore_POS;
}

void BLDC_AC_set(uint8_t Phase)
{
	switch (Phase)
	{
		case 1:
			if (reverse == 0)
				AC0.MUXCTRLA = BEMF_C_POS_AC | BEMF_NEG_AC;
			else
				AC0.MUXCTRLA = BEMF_B_POS_AC | BEMF_NEG_AC;
		break;
		case 2:
			if (reverse == 0)
				AC0.MUXCTRLA = BEMF_B_POS_AC | BEMF_NEG_AC;
			else
				AC0.MUXCTRLA = BEMF_C_POS_AC | BEMF_NEG_AC;
		break;
		case 3:
			AC0.MUXCTRLA = BEMF_A_POS_AC | BEMF_NEG_AC;
		break;
		case 4:
			if (reverse == 0)
				AC0.MUXCTRLA = BEMF_C_POS_AC | BEMF_NEG_AC;
			else
				AC0.MUXCTRLA = BEMF_B_POS_AC | BEMF_NEG_AC;
		break;
		case 5:
			if (reverse == 0)
				AC0.MUXCTRLA = BEMF_B_POS_AC | BEMF_NEG_AC;
			else
				AC0.MUXCTRLA = BEMF_C_POS_AC | BEMF_NEG_AC;
		break;
		case 6:
			AC0.MUXCTRLA = BEMF_A_POS_AC | BEMF_NEG_AC;
		break;
	}
}



//  ADC Wert einlesen und filtern
uint8_t ADC_read_and_filter(void)
{
	uint8_t	ADC_gefiltert;
	// Array Buffer muss so gross sein, wie die Anzahl Werte , die eingelesen werden
	uint8_t	Buffer[3];
	// Anzahl Werte, die eingelesen und gefiltert werden
	uint8_t Anzahl_Werte = 3;
	
	for(uint8_t i=0; i<Anzahl_Werte; i++)
	{		// starte ADC		ADC0.COMMAND = ADC_STCONV_bm;		// warte bis conversion abgeschlossen		while (ADC0.COMMAND & ADC_STCONV_bm){}
		Buffer[i]	=	ADC0.RESL;
	}
	ADC_gefiltert	=	Median_Filter_8Bit(Buffer,Anzahl_Werte);
	return ADC_gefiltert;
}

// Lese hier Mid_V
uint8_t ADC_read_Mid_V (void)
{
	uint8_t Mid_V;
	ADC0.MUXPOS = ADC_POS_MID;
	Mid_V	=	ADC_read_and_filter();
	return Mid_V;
}

// Lese hier BEMF_V
uint8_t ADC_read_BEMF_V (uint8_t Phase)
{
	uint8_t BEMF_V;
	switch (Phase)
	{
		case 1:
			if (reverse == 0)
				ADC0.MUXPOS = ADC_POS_BEMF_C;
			else
				ADC0.MUXPOS = ADC_POS_BEMF_B;			
		break;
		case 2:
			if (reverse == 0)
				ADC0.MUXPOS = ADC_POS_BEMF_B;
			else
				ADC0.MUXPOS = ADC_POS_BEMF_C;
		break;
		case 3:
			ADC0.MUXPOS = ADC_POS_BEMF_A;
		break;
		case 4:
			if (reverse == 0)
				ADC0.MUXPOS = ADC_POS_BEMF_C;
			else
				ADC0.MUXPOS = ADC_POS_BEMF_B;
		break;
		case 5:
			if (reverse == 0)
				ADC0.MUXPOS = ADC_POS_BEMF_B;
			else
				ADC0.MUXPOS = ADC_POS_BEMF_C;
		break;
		case 6:
			ADC0.MUXPOS = ADC_POS_BEMF_A;
		break;
	}
	BEMF_V = ADC_read_and_filter();
	return BEMF_V;
}

uint8_t BEMF_ADC_compare (uint8_t Phase, uint8_t Mid_V, uint8_t BEMF_V)
{
	uint8_t match = 0;
	// Phase gerade oder ungerade?
	if (Phase & 0x01)
	{
		if (Mid_V > BEMF_V)
			match = 1;
	}
	else
	{
		if (Mid_V < BEMF_V)
			match = 1;
	}		
	return match;
}

