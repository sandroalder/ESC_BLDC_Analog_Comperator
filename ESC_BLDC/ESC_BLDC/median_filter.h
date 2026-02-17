
// Median Filter
// Es handelt sich hier um ein Bubble sort Allgorythmus, 
// das dann den mittleren Wert aus dem Array zurück gibt.

// n = grösse von array
// Aus Effizienzgründen sind 8 und 16Bit getrennt


uint8_t Median_Filter_8Bit(uint8_t arr[], uint8_t n);

uint16_t Median_Filter_16Bit(uint16_t arr[], uint8_t n);


// Beispiel
/*

// Array Buffer muss so gross sein, wie die Anzahl Werte, die eingelesen werden

uint8_t	gefiltert;
uint8_t	Buffer[5];	
uint8_t Anzahl_Werte = 5;

gefiltert	=	Median_Filter(Buffer,5);

for(uint8_t i=0; i<Anzahl_Werte; i++)
{
	Buffer[i]	=	ADC Wert z.B.;
}
gefiltert	=	Median_Filter_8Bit(Buffer,Anzahl_Werte);

*/