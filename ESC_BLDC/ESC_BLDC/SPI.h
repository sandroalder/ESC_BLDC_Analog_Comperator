
#include <inttypes.h>

// SPI Kommunikation
// die Chip Select Ausgänge für den Host werden nicht hier konfiguriert

// Hier Port und Pins eintragen

#define PORT_MUX	PORTMUX_SPI0_DEFAULT_gc // PortA
//#define PORT_MUX	PORTMUX_SPI0_ALT1_gc	// PortC
//#define PORT_MUX	PORTMUX_SPI0_ALT2_gc	// PortE
					
#define SPI_PORT	PORTA
#define SPI_MOSI	PIN4_bm
#define SPI_MISO	PIN5_bm
#define SPI_SCK		PIN6_bm

// Nur für Client
#define SPI_SS		PIN7_bm


void SPI0_init_Host (void);
void SPI0_init_Client (void);

uint8_t SPI0_exchange_data (uint8_t data);
uint8_t SPI0_get_data (void);