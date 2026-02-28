
// Ansteuerung für einen 3Phasen BLDC Treiber

/*   - - \ _ _ /
	Phase 1 = Phase A -
	Phase 2 = Phase A -
	Phase 3 = Phase A \
	Phase 4 = Phase A _
	Phase 5 = Phase A _
	Phase 6 = Phase A /
*/


// Hier die Pinbelegung eintragen

/////////////////////////////////////////////////////////////////
// Motor_Driver

#define PWM_Port	PORTB
#define PWM			PIN0_bm

#define	Phase_Port	PORTC
#define INHA		PIN4_bm
#define INLA		PIN5_bm
#define INHB		PIN2_bm
#define INLB		PIN3_bm
#define INHC		PIN0_bm
#define INLC		PIN1_bm

void BLDC_set_reverse (uint8_t reverse_flag);

void BLDC_PWM_Init (void);

void BLDC_PWM_Set (uint16_t PWM_Wert); // Muss zwischen 0 und 511 liegen

void BLDC_Driver_init (void);

void BLDC_Stop (void);

void BLDC_Break (void);

uint8_t BLDC_get_ready (uint8_t Phase);

uint8_t BLDC_next_Phase (uint8_t Phase);

void BLDC_change_Phase (uint8_t Phase);


/////////////////////////////////////////////////////////////////
// Motor_BEMF

#define ADC_POS_MID		ADC_MUXPOS_AIN5_gc
#define ADC_POS_BEMF_A	ADC_MUXPOS_AIN4_gc
#define ADC_POS_BEMF_B	ADC_MUXPOS_AIN2_gc
#define ADC_POS_BEMF_C	ADC_MUXPOS_AIN1_gc

#define BEMF_Port		PORTD
#define BEMF_Mid		PIN5_bm
#define BEMF_A			PIN4_bm
#define BEMF_B			PIN2_bm
#define BEMF_C			PIN1_bm

#define BEMF_Mid_PCTRL	PIN5CTRL
#define BEMF_A_PCTRL	PIN4CTRL
#define BEMF_B_PCTRL	PIN2CTRL
#define BEMF_C_PCTRL	PIN1CTRL


/////////////////////////////////////////////////////////////////
// Motor_BEMF_Analog Comperator

#define BEMF_Port_AC		PORTD
#define BEMF_Mid_AC			PIN5_bm
#define BEMF_A_AC			PIN4_bm
#define BEMF_B_AC			PIN2_bm
#define BEMF_C_AC			PIN1_bm

#define BEMF_Mid_AC_PCTRL	PIN5CTRL
#define BEMF_A_AC_PCTRL		PIN4CTRL
#define BEMF_B_AC_PCTRL		PIN2CTRL
#define BEMF_C_AC_PCTRL		PIN1CTRL

#define BEMF_NEG_AC			AC_MUXNEG_PIN1_gc // N1 als Negativer Input für Mid_V
#define BEMF_A_POS_AC		AC_MUXPOS_PIN1_gc // Positiver Input muss immer geändert werden, da nur ein Comparator vorhanden
#define BEMF_B_POS_AC		AC_MUXPOS_PIN0_gc
#define BEMF_C_POS_AC		AC_MUXPOS_PIN3_gc



/////////////////////////////////////////////////////////////////

void BEMF_AC_init (void);

void BLDC_AC_set(uint8_t Phase);

//void BLDC_AC_ignore(uint8_t Phase);


void BEMF_ADC_init (void);

uint8_t ADC_read_Mid_V (void);

uint8_t ADC_read_BEMF_V (uint8_t Phase);

uint8_t BEMF_ADC_compare (uint8_t Phase, uint8_t Mid_V, uint8_t BEMF_V);

