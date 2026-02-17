/*
 * PWM_Protokolle.h
 *
 * Created: 05.02.2026 12:23:29
 *  Author: SA061401
 */ 

#include <inttypes.h>


#ifndef PWM_PROTOKOLLE_H_
#define PWM_PROTOKOLLE_H_



void PWM_shot_init (void);							// initialisieren

void PWM_shot_stop (void);	

void PWM_calibrate (uint16_t raw_pulse);			// Einmal an Anfang aufrufen, während Puls am kürzesten

uint16_t PWM_raw_read (void);


uint16_t Multishot_read (void);	

uint16_t Oneshot42_read (void);	




#endif /* PWM PROTOKOLLE_H_ */