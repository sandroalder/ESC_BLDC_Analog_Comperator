/*
 * median_filter.c
 *
 * Created: 09.08.2023 17:04:17
 * Author : SA061401
 */ 

#include <inttypes.h>
#include "median_filter.h"

uint8_t Median_Filter_8Bit(uint8_t arr[], uint8_t n) 
{
	uint8_t filtered_Value;
	uint8_t i, j;
	uint8_t temp;
	
	// Bubble sort
	for (i = 0; i < n - 1; i++) 
	{
		for (j = 0; j < n - i - 1; j++) 
		{
			if (arr[j] > arr[j+1])
			{
				temp		=	arr[j];
				arr[j]		=	arr[j+1];
				arr[j+1]	=	temp;
			}
		}
	}
	
	if (n & 0x01) // Wenn ungerade Anzahl
	{
			// gebe mittleren Wert zurück
			filtered_Value = arr[(n/2)];		
	}
	else
	{
			// gebe Durchschnitt der zwei mittleren Werte zurück
			filtered_Value = ((arr[(n/(2-1))] + arr[(n/2)]) / 2);		
	}
	return filtered_Value;

}

uint16_t Median_Filter_16Bit(uint16_t arr[], uint8_t n)
{
	uint16_t filtered_Value;
	uint8_t i, j;
	uint16_t temp;
	
	// Bubble sort
	for (i = 0; i < n - 1; i++)
	{
		for (j = 0; j < n - i - 1; j++)
		{
			if (arr[j] > arr[j+1])
			{
				temp		=	arr[j];
				arr[j]		=	arr[j+1];
				arr[j+1]	=	temp;
			}
		}
	}
	
	if (n & 0x01) // Wenn ungerade Anzahl
	{
		// gebe mittleren Wert zurück
		filtered_Value = arr[(n/2)];
	}
	else
	{
		// gebe Durchschnitt der zwei mittleren Werte zurück
		filtered_Value = ((arr[(n/(2-1))] + arr[(n/2)]) / 2);
	}
	return filtered_Value;
}