
/* BLDC Controller
 * ESC_BLDC.cpp
 *
 * Created: 30.01.2026 17:04:17
 * Author : SA061401
 */ 


/*   - - \ _ _ /
	Phase 1 = Phase A -
	Phase 2 = Phase A -
	Phase 3 = Phase A \
	Phase 4 = Phase A _
	Phase 5 = Phase A _
	Phase 6 = Phase A /
*/

// SPI input es sollte mindestens 1ms gewartet werden beim Senden der Parameter, 
// damit diese auch sicher eingelesen werden
// Um das reverse Flag zu setzen muss zuerst ein BLDC_Stop gesendet werden
/*
0		=	BLDC_Stop
1		=	BLDC_break
2		=	BLDC_reverse_off
3		=	BLDC_reverse_on
4		=	K_Winkel 0°
5		=	K_Winkel 5°
6		=	K_Winkel 10°
7		=	K_Winkel 15°
8		=	K_Winkel 20°
9		=	K_Winkel 25°
10		=	K_Winkel 30°
11-30	=	reserved
31-255	=	PWM_einstellen
*/

#include <avr/io.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#define	F_CPU 20000000UL
#include <util/delay.h>


#include "System_Init.h"
#include "BLDC_Driver.h"
#include "PWM_Protokolle.h"
#include "digital_shot.h"
#include "moving_avarage_filter.h"
//#include "SPI.h"



//////////////////////////////////////////////////////////////
// Parameter

// Zeit eintragen, die der Analog Komparator nach dem Kommutieren ausgeschaltet sein soll in zehntel uS
#define AC_Ignore_uS	300

// Kommutierungswinkel in Grad eintragen
//volatile uint8_t K_Winkel = 10;//25;
#define K_Winkel	30
// Zeit in zehnteluS, die gebraucht wird um die Phasen umzuschalten
#define T_Konst		100
// max Zeit für die Winkelkommutierung in zehnteluS
#define max_Time	5000
// Es wird eine min. Zeit benötigt, damit es funktioniert in zehnteluS
#define min_Time	80
// Start Zeit für Zwangskommutierung
#define start_Time	50

// Reverse Flag
volatile uint8_t reverse_flag = 0;


//////////////////////////////////////////////////////////////
// globale variable / wird in Interrupt Routine gebraucht
// volatile sollte verwendet werden, wenn variable in Interrupt Routine verwendet wird
volatile uint16_t Timer_Drehzahl = 0xFFFF;
volatile uint16_t Kommutierung = 0;

volatile uint8_t Phase = 1;
volatile bool Kommutiere = false;
volatile bool AC_ignore = false;

/////////////////////////////////////////////////////////////////
// Kommutirungswinkel

// Timer B 0/1 init mit compare match Interrupt für Drehzahlmessung
// hier wird eine Pseudo Drehzahl gemessen zur Berechnung des Kommutierungswinkels


void LUT_enable(void)
{
	CCL.SEQCTRL0	= CCL_SEQSEL0_DFF_gc        /* D FlipFlop */
					| CCL_SEQSEL1_DISABLE_gc;	/* Sequential logic disabled */

	CCL.TRUTH0 = 2;

	CCL.LUT0CTRLB	= CCL_INSEL0_AC0_gc			/* AC0 OUT input source */
					| CCL_INSEL1_MASK_gc;		/* Masked input */

	CCL.LUT0CTRLA	= CCL_CLKSRC_CLKPER_gc		/* CLK_PER is clocking the LUT */
					| CCL_EDGEDET_DIS_gc		/* Edge detector is disabled */
					| CCL_FILTSEL_FILTER_gc		/* Filter disabled */
					| 1 << CCL_ENABLE_bp		/* LUT Enable: enabled */
					| 1 << CCL_OUTEN_bp;		/* Output Enable: enabled */

	CCL.TRUTH1 = 1;

	CCL.LUT1CTRLC	= CCL_INSEL2_TCB2_gc;		/* TCB2 WO input source */

	CCL.LUT1CTRLA	=	CCL_CLKSRC_CLKPER_gc    /* CLK_PER is clocking the LUT */
					| CCL_EDGEDET_DIS_gc		/* Edge detector is disabled */
					| CCL_FILTSEL_DISABLE_gc	/* Filter disabled */
					| 1 << CCL_ENABLE_bp		/* LUT Enable: enabled */
					| 0 << CCL_OUTEN_bp;		/* Output Enable: disabled */

	CCL.INTCTRL0 = CCL_INTMODE0_BOTH_gc;		// Interrupt on both Edges

	//CCL.CTRLA		= 1 << CCL_ENABLE_bp		/* Enable: enabled */
	//				| 0 << CCL_RUNSTDBY_bp;		/* Run in Standby: disabled */
}

void BLDC_C_Angle_init (void)
{
	//ein increment alle 0.1uS bei einem 20MHz Takt
	TCB0.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
	TCB1.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
	
	LUT_enable();

	//EVSYS.CHANNEL2 = EVSYS_GENERATOR_AC0_OUT_gc;
	EVSYS.CHANNEL2 = EVSYS_GENERATOR_CCL_LUT0_gc;
	EVSYS.USERTCB1 = EVSYS_CHANNEL_CHANNEL2_gc;
	EVSYS.USERTCB0 = EVSYS_CHANNEL_CHANNEL3_gc;

	TCB0.CTRLB |= TCB_CNTMODE_FRQ_gc;
	TCB0.EVCTRL |= TCB_CAPTEI_bm;
	TCB0.EVCTRL &= ~TCB_EDGE_bm;
	//TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CTRLA |= TCB_ENABLE_bm;
	
	// Single shot mode
	TCB1.CTRLB |= TCB_CNTMODE_SINGLE_gc;
	TCB1.EVCTRL |= TCB_CAPTEI_bm;
	TCB1.EVCTRL |= TCB_EDGE_bm;
	TCB1.INTCTRL |= TCB_CAPT_bm;
	TCB1.CTRLA |= TCB_ENABLE_bm;
}

void AC_Ignore_init (void)
{
	EVSYS.CHANNEL3 = EVSYS_GENERATOR_TCB1_CAPT_gc;
	EVSYS.USERTCB2 = EVSYS_CHANNEL_CHANNEL3_gc;
	//EVSYS.USERTCB2 = EVSYS_CHANNEL_CHANNEL2_gc;
	
	TCB2.CCMP = AC_Ignore_uS;
	TCB2.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
	
	// Single shot mode
	TCB2.CTRLB |= TCB_CNTMODE_SINGLE_gc;
	TCB2.EVCTRL |= TCB_CAPTEI_bm;
	TCB2.EVCTRL |= TCB_EDGE_bm;
	TCB2.INTCTRL |= TCB_CAPT_bm;
	TCB2.CTRLA |= TCB_ENABLE_bm;
}


volatile bool rechne = false;
volatile uint16_t Time_Delay = min_Time;
void berechne_K_Winkel(void)
{
	uint16_t Drehzahl;
	uint16_t tmp;
	
	if (rechne)
	{												
		rechne = false;
		Drehzahl = Moving_Avarage_Filter_16Bit_1(TCB0.CCMP);

		tmp = ((uint32_t)Drehzahl * K_Winkel) / 60;
	
		if (tmp > T_Konst)
			tmp -= T_Konst;
		if (tmp < min_Time)
			tmp = min_Time;
		if (tmp > max_Time)
			tmp = max_Time;
			
		cli();
		Time_Delay = tmp;
		sei();
	}
}


ISR (TCB1_INT_vect)
{									
	uint8_t tmp;

	tmp = BLDC_next_Phase(Phase);
	BLDC_AC_set(tmp);
	BLDC_change_Phase(tmp);
	Phase = tmp;
	
	rechne = true;
	
	//_delay_us(1);
	
	TCB1.CNT = TCB1.CCMP;
	TCB1.INTFLAGS = TCB_CAPT_bm;												PORTA.OUTCLR = PIN4_bm;
}

ISR (TCB2_INT_vect)
{
	TCB2.INTFLAGS = TCB_CAPT_bm;
}


/////////////////////////////////////////////////////////////////
// Analog Comparator Interrupt 
// Kommt wenn BEMF Spannung Mittelpunktspannung überschritten hat

ISR(AC0_AC_vect)
{
	AC0.STATUS = AC_CMP_bm;
}

ISR(CCL_CCL_vect)
{												PORTA.OUTSET = PIN4_bm;
	BLDC_AC_set(BLDC_next_Phase(Phase));
	TCB1.CCMP = Time_Delay;
	CCL.INTFLAGS = CCL_INT0_bm;		// Lösche Interrupt Flag
}

/////////////////////////////////////////////////////////////////
// Zwangskommutierung	
void Zwangskommutierung (void)
{

	//uint8_t Mid_V = 0;
	//uint8_t Bemf_V = 0;
	
	BLDC_PWM_Set (0);
	Moving_Avarage_Filter_16Bit_1_preset(start_Time); // Startwert
	TCB1.CCMP = start_Time;
	Time_Delay = start_Time;
	
	Phase = 1;
	Phase = BLDC_get_ready(Phase);	// Setzt alle Output Pins
	BLDC_change_Phase(Phase);
	_delay_ms(1);

	// Anlaufen
	BLDC_PWM_Set(100);
	_delay_ms(200);
	BLDC_PWM_Set(250);
	
	//Mid_V	=	ADC_read_BEMF_V(3); // Messe hier Phase A und bilde Mid_V ab
	
	uint16_t wait = 6000;
	for (uint16_t i = 0; i < 160; i++)
	{
		Phase = BLDC_next_Phase(Phase);
		BLDC_change_Phase(Phase);
		for (uint16_t t = 0; t < wait; t++)
		{
			_delay_us(1);
		}
		wait -= 35; 
	}
	
	//while (BEMF_ADC_compare (Phase, Mid_V, ADC_read_BEMF_V(Phase))) {}
	
	/*
	for (uint16_t i = 0; i<10000; i++)
	{
		Bemf_V	=	ADC_read_BEMF_V(Phase);
		if (BEMF_ADC_compare (Phase, Mid_V, Bemf_V))
		{PORTA.OUTTGL = PIN6_bm;
			BLDC_next_Phase(Phase);
			BLDC_change_Phase(Phase);
		}
		_delay_us(10);
	}*/
	
	// Aktiviert AC und setzt ihn auf entsprechenden Input
	BLDC_AC_set(Phase);
	CCL.CTRLA = 1 << CCL_ENABLE_bp;

	_delay_ms(400);
}


/////////////////////////////////////////////////////////////////
// normal operation
void normal_operation (void)
{
	berechne_K_Winkel();
	_delay_us(10);
}


/////////////////////////////////////////////////////////////////
// main
int main(void)
{					PORTA.DIRSET = PIN4_bm; PORTMUX.CCLROUTEA = PORTMUX_LUT0_bm;
	bool Multishot = false;
	bool dshot = false;
	
	bool Start = false;
	bool run = false;
	
	uint16_t cnt_stop = 0;
	
	uint16_t PWM_Wert = 0;

	clock_init();
	_delay_ms(20);			//Warte bis clock und Supply stabil
	
	PWM_shot_init();
	_delay_ms(20);			// Nötig, damit der erste Puls gemessen wird bevor es weiter geht
	uint16_t PWM_raw = PWM_raw_read();
	PWM_calibrate(PWM_raw);
	
	if (PWM_raw >= 90 && PWM_raw <= 110)	// Taktzyklen
		Multishot = true;

	if (PWM_raw >= 45 && PWM_raw <= 55)		// Wenn digitales Protokoll verwendet, dann konfiguriere um 
		dshot = true;
	if (dshot)
	{
		PWM_shot_stop();
		dshot_init();
	}
		
	BLDC_Driver_init();
	BLDC_Stop();
	BLDC_PWM_Init();
	BLDC_C_Angle_init();
	//BEMF_ADC_init();
	BEMF_AC_init();
	AC_Ignore_init();
	//SPI0_init_Client();
	
	BLDC_set_reverse(reverse_flag);
	
	sei(); // interrupt enable
	
	while (1)
	{
		
		//{
		// Mehr Zeug könnte hier gemacht werden
		//}
		
		if (Multishot)
			PWM_Wert = Multishot_read() >> 2;	// Zwei nach rechts, da Wert von 0-2047 
		if (dshot)								// zurückgegeben wird, aber er von 0-511 gehen muss
			PWM_Wert = dshot_read() >> 2;
			
		// Bei weniger als ~15% PWM stoppt der Motor
		if (PWM_Wert >= 80)
		{
			Start = true;
			cnt_stop = 0;	
		}
		else
		{
			cnt_stop++;
			if (cnt_stop >= 2000)
			{
				CCL.CTRLA = 0 << CCL_ENABLE_bp;
				BLDC_Stop();
				
				cnt_stop = 0;
				run = false;
				Start = false;
			}
		}
		
		if (run)
		{
			BLDC_PWM_Set (PWM_Wert);
			normal_operation();
		}
		else
		{
			// Zwangskommutierung
			if (Start)
			{
				Zwangskommutierung();  	
				run = true;
			}
		}
	}
}

