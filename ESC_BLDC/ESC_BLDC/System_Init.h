// Hier werden Systemrelevante Parameter konfiguriert


#define CLK_SEL		CLKCTRL_CLKSEL_OSC20M_gc	//INTERNER 20MHz CLK
//#define CLK_SEL		CLKCTRL_CLKSEL_EXTCLK_gc	//Externer CLK

#define CLK_PRSC	0							//Prescaler aus
//#define CLK_PRSC	1							//Prescaler 2


void clock_init (void);

//void set_brown_out (void);

//void set_read_protection (void);