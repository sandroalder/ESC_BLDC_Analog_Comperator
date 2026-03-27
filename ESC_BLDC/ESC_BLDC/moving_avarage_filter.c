/*
 * moving_avarage_filter.c
 *
 * Created: 09.08.2023 17:04:17
 * Author : SA061401
 */ 


#include <inttypes.h>
#include "moving_avarage_filter.h"


uint16_t Moving_Avarage_Filter_16Bit_1(uint16_t new_value)
{
	static uint16_t buffer[n_1];
	static uint8_t i;
	uint32_t sum;
	
	sum -= buffer[i];		// alten Wert entfernen
	buffer[i] = new_value;	// neuen Wert speichern
	sum += new_value;		// neuen Wert addieren

	i++;
	if(i >= n_1)
		i = 0;

	return sum / n_1;
}