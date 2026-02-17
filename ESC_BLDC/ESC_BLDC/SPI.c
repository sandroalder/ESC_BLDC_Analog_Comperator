/*
 * SPI.c
 *
 * Created: 31.08.2023 15:21:21
 *  Author: SA061401
 */ 

#include <avr/io.h>
#include <inttypes.h>
#include "SPI.h"

void SPI0_init_Host (void)
{
	SPI_PORT.DIR	|=	SPI_MOSI;
	SPI_PORT.DIR	&=	~SPI_MISO;
	SPI_PORT.DIR	|=	SPI_SCK;
	
	PORTMUX.TWISPIROUTEA |= PORT_MUX;
	
	// Das MSB wird zuerst geschickt
	SPI0.CTRLA	&=	~SPI_DORD_bm; 
	// Master Mode
	// clock 1.25MHz bei 20MHz Takt
	SPI0.CTRLA	|=	SPI_MASTER_bm
				|	SPI_PRESC_DIV16_gc;	
	
	// Buffer ausgeschaltet
	// Slave select in Master mode ausschalten
	SPI0.CTRLB	|=	SPI_BUFWR_bm
				|	SPI_SSD_bm;
				
	SPI0.CTRLA	|=	SPI_ENABLE_bm;
}


void SPI0_init_Client (void)
{
	SPI_PORT.DIR	&=	~SPI_MOSI;
	SPI_PORT.DIR	|=	SPI_MISO;
	SPI_PORT.DIR	&=	~SPI_SCK;
	SPI_PORT.DIR	&=	~SPI_SS;
	
	PORTMUX.TWISPIROUTEA |= PORT_MUX;
	
	// Das MSB wird zuerst geschickt
	// Slave Mode
	SPI0.CTRLA	&=	~SPI_DORD_bm
				&	~SPI_MASTER_bm;
	
	// Buffer ausgeschaltet
	//SPI0.CTRLB	&=	~SPI_BUFEN_bm;
		
	SPI0.CTRLA	|=	SPI_ENABLE_bm;
}

// Send
uint8_t SPI0_exchange_data (uint8_t data)
{
	SPI0.DATA = data;
	while (!(SPI0.INTFLAGS & SPI_IF_bm)){};
	return SPI0.DATA;
}


// empfange Daten
uint8_t SPI0_get_data (void)
{
	return SPI0_DATA;	
}