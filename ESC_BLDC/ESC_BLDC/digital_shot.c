
/*
 * digital_shot.c
 *
 * Created: 02.02.2026 13:58:52
 *  Author: SA061401
 */ 



// Digital shot kommunikation. Nur dshot150 möglich mit ATmega4809 => Rechenleistung reicht nicht aus

// Hier die Pinbelegung eintragen

/////////////////////////////////////////////////////////////////

// Konfiguriere hier die Parameter

// mit prescaler 1 ein increment alle 0.05uS bei einem 20MHz Takt / max. Pulse 3’276.8uS
#define CLK_select	TCB_CLKSEL_CLKDIV1_gc			// für d_shot150

#define TCBx		TCB3							// Timer counterB3
#define	TCBx_vect	TCB3_INT_vect
#define USERTCBx	USERTCB3						// Timer counterB3

#define CHANNELx	EVSYS.CHANNEL1					// Channel 1
#define EV_CHANNEL	EVSYS_CHANNEL_CHANNEL1_gc		// Channel 1

#define PORTx_PINx	EVSYS_GENERATOR_PORT0_PIN7_gc	// PortA Pin7
//#define PORTx_PINx	EVSYS_GENERATOR_PORT1_PIN2_gc	// PortB Pin2

#define Bit_Mitte	75								//in Taktzyklen
#define Time_out	200								//soll etwa 1.5 Bit Längen sein in Taktzyklen
//#define Frame		1000							//in Taktzyklen



#include "digital_shot.h"

#include <inttypes.h>
#include <stdlib.h>
#include <avr/interrupt.h>


static volatile uint16_t	d_shot_crc = 0;

ISR(TCBx_vect)	
{
	//TCBx.INTFLAGS = TCB_CAPT_bm; // lösche Interrupt flag
	static  uint16_t	d_shot_buff = 0;	// static sorgt dafür, dass der Wert erhalten bleibt
	static  uint8_t		d_shot_cnt = 0;
	
	uint16_t tmp_CCMP = TCBx.CCMP;			// Zugriffe auf TCBx.CCMP dauern länger
											// Den Wert zwischenzuspeichern optimiert die Laufzeit
	d_shot_buff <<= 1;
	if (tmp_CCMP < Bit_Mitte)
	d_shot_buff++;

	if (++d_shot_cnt == 15)
	{
		d_shot_crc = d_shot_buff;
		d_shot_cnt = 0;
	}	
	if (tmp_CCMP > Time_out)
		d_shot_cnt = 0;
}


uint16_t dshot_read (void)
{
	static uint16_t	d_shot_reg = 0;
	
	uint16_t frame;
	uint16_t data;
	cli();							//blockt Interrupt solange Wert ausgelesen wird
	frame = d_shot_crc << 1;
	sei();
		data = frame >> 4;			// 12 Bit: throttle + telemetry
		// Überprüfe Checksumme => Verwerfe letztes Bit, da DShot ultrascheisse ist
		// und Dieser Timer nicht dafür geeignet
		if ((frame & 0x0E) == ((data ^ (data >> 4) ^ (data >> 8)) & 0x0E))
		{
			d_shot_reg = (frame >> 5) & 0x07FF;
		}
	return d_shot_reg;
}



void dshot_init (void)
{
	    // Setze Port 1 Pin 2 (PB2) als input event
	    CHANNELx = PORTx_PINx;
	    // Verbinde zu event channel 0
	    EVSYS.USERTCBx = EV_CHANNEL;
		    
		// Aktviere TCB ohne Prescaler 
		TCBx.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
		    
		// Messe Puls
		TCBx.CTRLB = TCB_CNTMODE_PW_gc;
		    
		// Setze Capture interrupt
		TCBx.INTCTRL = TCB_CAPT_bm;
		   
		// Setze Event Input und triggere auf negativen Puls
		TCBx.EVCTRL	|= TCB_CAPTEI_bm;
		TCBx.EVCTRL	|= TCB_EDGE_bm;
}

void dshot_stop (void)
{
	TCBx.CTRLA  &= ~TCB_ENABLE_bm;
}