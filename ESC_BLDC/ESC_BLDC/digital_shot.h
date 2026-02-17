
#include <inttypes.h>


#ifndef DIGITAL_SHOT_H_
#define DIGITAL_SHOT_H_



void dshot_init (void);								// initialisieren

//void dshot1200_init (void);

void dshot_stop (void);	


uint16_t dshot_read (void);							// Protokoll lesen


#endif