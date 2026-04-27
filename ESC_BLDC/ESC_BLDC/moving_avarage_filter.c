/*
 * moving_avarage_filter.c
 *
 * Created: 09.08.2023 17:04:17
 * Author : SA061401
 */ 


#include <inttypes.h>
#include "moving_avarage_filter.h"



static uint16_t buffer_1[n_1];
static uint32_t sum_1;

void Moving_Avarage_Filter_16Bit_1_preset(uint16_t value)
{
	for (uint8_t i = 0; i < n_1; i++)
	{
		buffer_1[i] = value;
	}
	sum_1 = value * n_1;
}

uint16_t Moving_Avarage_Filter_16Bit_1(uint16_t new_value)
{
	
	static uint8_t i;
	
	sum_1 -= buffer_1[i];		// alten Wert entfernen
	buffer_1[i] = new_value;	// neuen Wert speichern
	sum_1 += new_value;			// neuen Wert addieren

	i++;
	if(i >= n_1)
		i = 0;

	return sum_1 / n_1;
}