
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

#define	F_CPU 20000000UL
#include <util/delay.h>

// extern "C" wird benötigt für die Einbindung von c files in ein c++ file
//extern "C" 
//{	
//}

#include "System_Init.h"
#include "BLDC_Driver.h"
#include "PWM_Protokolle.h"
#include "digital_shot.h"
//#include "SPI.h"


//////////////////////////////////////////////////////////////
// Parameter

// Kommutierungswinkel in Grad eintragen
volatile uint8_t K_Winkel = 0;//25;
volatile uint8_t reverse_flag = 0;


//////////////////////////////////////////////////////////////
// globale variable / wird in Interrupt Routine gebraucht
// volatile sollte verwendet werden, wenn variable in Interrupt Routine verwendet wird
volatile uint16_t Timer_Drehzahl = 0xFFFF;
volatile uint16_t Kommutierung = 0;

volatile uint8_t Phase = 1;
volatile bool Kommutiere = false;


/////////////////////////////////////////////////////////////////
// Kommutirungswinkel

// Timer B 0/1 init mit compare match Interrupt für Drehzahlmessung
// hier wird eine Pseudo Drehzahl gemessen zur Berechnung des Kommutierungswinkels
void BLDC_C_Angle_init (void)
{
	// mit prescaler 2 ein increment alle 0.1uS bei einem 20MHz Takt
	TCB0.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;
	// compare match value
	TCB0.CCMP = 100;	// aktiviere Interrupt	// TCB0.INTCTRL = TCB_CAPT_bm;
	// starte Timer
	TCB0.CTRLA |= TCB_ENABLE_bm;
	// mit prescaler 2 ein increment alle 0.1uS bei einem 20MHz Takt
	TCB1.CTRLA |= TCB_CLKSEL_CLKDIV2_gc;	// compare match value	TCB1.CCMP = 100;
}

// ISR kommt alle 5uS. Dann wird Variable um 1 erhöht um die Drehzahl zu ermitteln
ISR (TCB0_INT_vect)
{	
	TCB0.INTFLAGS = TCB_CAPT_bm;
	if (Timer_Drehzahl < 0xFFFF)
		Timer_Drehzahl ++;
}
// ISR kommt auch alle 5uS nachdem Timer nach ADC match gestartet wurde
ISR (TCB1_INT_vect)
{
	static uint16_t Timer_Kommutierung;
	// Prüfe ob Winkel erreicht wurde
	if (Kommutierung == Timer_Kommutierung)
	{
		//Kommutiere = true;
		Timer_Kommutierung = 0;
		TCB1.CTRLA &= ~TCB_ENABLE_bm; // Stoppt Timer
	}
	Timer_Kommutierung ++;
}


/////////////////////////////////////////////////////////////////
// Analog Comparator Interrupt 
// Kommt wenn BEMF Spannung Mittelpunktspannung überschritten hat

ISR(AC0_AC_vect)
{	PORTA.OUTTGL = PIN4_bm;
	
	uint8_t tmp;		// braucht wenier Taktzyklen mit Zwischenspeicher
	// gehe in nächste Phase über
	tmp = BLDC_next_Phase(Phase);
	BLDC_AC_set(tmp);
	BLDC_change_Phase(tmp);
	Phase = tmp;
	
	AC0.STATUS = AC_CMP_bm;		// Lösche Interrupt Flag
	// starte Timer für K Winkels
	// TCB1.CTRLA |= TCB_ENABLE_bm;
	// Kommutiere = true; // Test mit Direktkommutierung 
}


/////////////////////////////////////////////////////////////////
// Zwangskommutierung	
void Zwangskommutierung (void)
{
	uint16_t Drehzahl = 0xFFFF;
	
	//uint8_t Mid_V = 0;
	//uint8_t BEMF_V = 0;
	//uint8_t match = 0;
	
	uint16_t cnt = 0;
	uint8_t PWM_full = 0;
	
	BLDC_PWM_Set (0);
	Phase = BLDC_get_ready(Phase);	// Setzt alle Output Pins
	_delay_ms(1);					// Warte bis Bemf geladen. Sonst kommt AC Interrupt sofort
	BLDC_AC_set(Phase);				// Aktiviert AC und setzt ihn auf entsprechenden Input
	AC0.STATUS = AC_CMP_bm;			// Setze Interrupt Flag zurück
	AC0.INTCTRL = AC_CMP_bm;
	
	Kommutierung = 1;				// Setzt Winkelkommutierung auf 0°

	// Anlaufen
	while (0)//(Drehzahl  > 800)
	{
		if (cnt >= 1500)
		{
			if (PWM_full)
			{
				BLDC_PWM_Set(50);
				PWM_full = 0;
			}
			else
			{
				BLDC_PWM_Set(450);
				PWM_full = 1;
			}
			cnt = 0;
		}
		cnt ++;
		_delay_us(10);
		
		if (Kommutiere)
		{
			// gehe in nächste Phase über
			//Phase = BLDC_next_Phase(Phase);
			//BLDC_change_Phase(Phase);
			
			
			// Immer wenn alle 6 Phasen durch sind, aktualisiere Drehzahl
			if (Phase == 1)
			{
				Drehzahl = Timer_Drehzahl;
				Timer_Drehzahl = 0;
			}
			Kommutiere = false;
		}
	
		/*// Messe und vergleiche BEMF mit Mittelpunktspannung
		Mid_V	=	ADC_read_Mid_V();
		BEMF_V	=	ADC_read_BEMF_V(Phase);
		match	=	BEMF_ADC_compare(Phase, Mid_V, BEMF_V);

		if (match)
		{
			Kommutiere = true;
			match = 0;
		}*/
	}
}


/////////////////////////////////////////////////////////////////
// normal operation
void normal_operation (void)
{
	uint16_t Drehzahl = 0xFFFF;		// Ist die Zeit in uS zwischen allen 6 Phasen
	
	//uint8_t Mid_V = 0;
	//uint8_t BEMF_V = 0;
	//uint8_t match = 0;
	
	// Kommutiere
	if (Kommutiere)
	{
		Kommutiere = false;

		// BLDC_AC_set(Phase);
			
		// Immer wenn alle 6 Phasen durch sind, aktualisiere Drehzahl
		if (Phase == 1)
		{
			Drehzahl = Timer_Drehzahl;
			Timer_Drehzahl = 0;
			// Berechne Kommutierung
			Kommutierung = (Drehzahl*K_Winkel)/360;
			
		}
		
	}
	
	/*// Messe und vergleiche BEMF mit Mittelpunktspannung
	Mid_V	=	ADC_read_Mid_V();
	BEMF_V	=	ADC_read_BEMF_V(Phase);
	match	=	BEMF_ADC_compare(Phase, Mid_V, BEMF_V);

	if (match)
	{		//Kommutiere = true;  // Test!!!!!! wenn Winkelkommutierung entfernt
		Kommutiere = true;
		match = 0;
	}*/
}


/////////////////////////////////////////////////////////////////
// main
int main(void)
{					PORTA.DIRSET = PIN4_bm;
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
	// BEMF_ADC_init();
	BEMF_AC_init();
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
			
		// Bei weniger als ~10% PWM stoppt der Motor
		if (PWM_Wert >= 40)
		{
			Start = true;
			cnt_stop = 0;	
		}
		else
		{
			cnt_stop++;
			if (cnt_stop >= 2000)
			{
				//AC0.INTCTRL &= ~AC_CMP_bm;
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

